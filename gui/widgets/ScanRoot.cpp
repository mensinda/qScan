#include "ScanRoot.hpp"
#include "util/QImageContainer.hpp"
#include "MainWindow.hpp"
#include <QMessageBox>
#include <QTabBar>
#include <chrono>
#include <future>

#include "ImagesTab.hpp"
#include "qscan_log.hpp"
#include "ui_ScanRoot.h"

using namespace qscan::lib;
using namespace std::chrono_literals;

namespace qscan::gui {

ScanRoot::ScanRoot(QWidget *parent) : QWidget(parent), ui(new Ui::ScanRoot), progressTimer(parent) {
    ui->setupUi(this);

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->lScanInfo->hide();

    progressTimer.setInterval(25);

    delayScanUntil = std::chrono::time_point<std::chrono::system_clock>::min();
    delayScanStart = std::chrono::time_point<std::chrono::system_clock>::min();

    QObject::connect(&progressTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    QObject::connect(this, SIGNAL(scanHasFinished()), this, SLOT(handleScanDone()));
    QObject::connect(this, SIGNAL(scanHasFailed()), this, SLOT(handleScanFailed()));
    QObject::connect(this, SIGNAL(signalConnected()), this, SLOT(deviceConnected()));
    QObject::connect(this, SIGNAL(signalConnectionFailed()), this, SLOT(connectionFailed()));
}

ScanRoot::~ScanRoot() {
    if (connectFuture.valid()) {
        connectFuture.get();
    }
    if (scanFuture.valid()) {
        (void)scanFuture.get();
    }
    if (mainWindow->config().lastDevice) {
        mainWindow->config().lastDevice->options = saneDevice->optionsSnapshot();
    }
}

QImageContainer ScanRoot::doScan() {
    while (delayScanUntil > std::chrono::system_clock::now() && numPendingScans > 0) {
        // This is not 100% accurate, but definitely good enough since the
        // total delay is measured in seconds anyway...
        std::this_thread::sleep_for(25ms);
    }

    // The scan was canceled during the delay
    if (numPendingScans <= 0) {
        lastException = SaneException(SANE_STATUS_CANCELLED, "The scan was canceled");
        emit scanHasFailed();
        return {};
    }

    try {
        SaneImage img = saneDevice->scan();
        auto      raw = img.asRGBA8();

        QImage qtImage{
            const_cast<const uchar *>(reinterpret_cast<uchar *>(raw.get())),
            (int)img.width(),
            (int)img.height(),
            QImage::Format_RGBA8888,
        };

        emit scanHasFinished();
        return {std::move(raw), {}, std::move(qtImage)};
    } catch (const SaneException &e) {
        lastException = e;
        emit scanHasFailed();
    } catch (const std::exception &e) {
        lastException = SaneException(SANE_STATUS_INVAL, e.what());
        emit scanHasFailed();
    }
    return {};
}

void ScanRoot::guiStateScanning() {
    ui->scanProgress->setEnabled(true);
    ui->scanProgress->setMinimum(0);
    ui->scanProgress->setMaximum(0);
    ui->cfgRootWidget->setEnabled(false);
    ui->btnPreview->setEnabled(false);
    ui->btnScanOne->setEnabled(false);
    ui->btnScanBatch->setEnabled(false);
    ui->btnStop->setEnabled(true);
    ui->mainTabs->setEnabled(false);
    ui->lScanInfo->show();

    progressTimer.start();
}

void ScanRoot::guiStateReady() {
    progressTimer.stop();

    ui->scanProgress->setEnabled(false);
    ui->scanProgress->setValue(0);
    ui->cfgRootWidget->setEnabled(true);
    ui->btnPreview->setEnabled(true);
    ui->btnScanOne->setEnabled(true);
    ui->btnScanBatch->setEnabled(true);
    ui->btnStop->setEnabled(false);
    ui->mainTabs->setEnabled(true);
    ui->lScanInfo->hide();
}

void ScanRoot::handleScanDone() {
    auto *tab = currentTab();
    if (tab == nullptr) {
        auto [_a, _b, qtImage] = scanFuture.get();
        ui->preview->updateImage(qtImage);
    } else {
        tab->addImage(scanFuture.get());
        mainWindow->setCanSave(true);
    }

    if (--numPendingScans > 0) {
        int delay = ui->settings->batchConfig().delay;
        if (delay > 0) {
            delayScanStart = std::chrono::system_clock::now();
            delayScanUntil = delayScanStart + std::chrono::seconds(delay);
        } else {
            delayScanUntil = delayScanStart = std::chrono::time_point<std::chrono::system_clock>::min();
        }
        scanFuture = std::async(std::launch::async, &ScanRoot::doScan, this);

        // Return early to continue
        return;
    }

    // The scan process has finished
    numPendingScans = 0; // Should not be required but just to be sure...
    if (!optionsSnapshot.empty()) {
        saneDevice->applyOptionSnapshot(optionsSnapshot);
        optionsSnapshot.clear();
    }

    ui->settings->reloadOptions();
    guiStateReady();
}

void ScanRoot::handleScanFailed() {
    guiStateReady();
    numPendingScans = 0;
    if (!optionsSnapshot.empty()) {
        saneDevice->applyOptionSnapshot(optionsSnapshot);
        optionsSnapshot.clear();
    }

    logger()->error("Scan failed with: {}", lastException.what());

    switch (lastException.saneStatus()) {
        case SANE_STATUS_GOOD: break;
        case SANE_STATUS_CANCELLED:
            QMessageBox::information(this, tr("Scan canceled"), tr("The scan process was canceled"));
            break;
        case SANE_STATUS_NO_DOCS:
            QMessageBox::information(this, tr("ADF empty"), tr("The automatic document feeder is out of documents"));
            break;
        case SANE_STATUS_JAMMED:
            QMessageBox::critical(this, tr("ADF jammed"), tr("The automatic document feeder is jammed"));
            break;
        case SANE_STATUS_COVER_OPEN:
            QMessageBox::warning(this, tr("Lid open"), tr("The scanner lid is open. Please close it and try again"));
            break;
        default:
            QMessageBox::critical(this,
                                  tr("Scanning failed"),
                                  tr("<b>Failed to scan:</b><br>%1<br><br><code>%2</code>")
                                      .arg(QString::fromStdString(saneDevice->getName()))
                                      .arg(lastException.what()));
            break;
    }
}

void ScanRoot::scanPreview() {
    guiStateScanning();

    optionsSnapshot = saneDevice->optionsSnapshot();

    auto &opts = saneDevice->getOptions();
    if (opts.getPreview().has_value()) {
        opts.setPreview(false);
    }

    auto resolutionOpts = opts.getResolution();
    if (resolutionOpts.has_value() && !resolutionOpts->values.empty()) {
        auto min = std::ranges::min_element(resolutionOpts->values);
        opts.setResolution(*min);
    }

    auto scanAreaOpt = opts.getScanArea();
    if (scanAreaOpt.has_value()) {
        opts.setScanArea({
            scanAreaOpt->min.topLeftX,
            scanAreaOpt->min.topLeftY,
            scanAreaOpt->max.bottomRightX,
            scanAreaOpt->max.bottomRightY,
        });
    }

    ui->mainTabs->setCurrentIndex(0);
    numPendingScans = 1;
    scanFuture      = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanOne() {
    // New tab when on preview
    if (ui->mainTabs->currentIndex() == 0) {
        newTab();
    }
    guiStateScanning();
    numPendingScans = 1;
    scanFuture      = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanBatch() {
    // New tab when on preview
    if (ui->mainTabs->currentIndex() == 0) {
        newTab();
    }

    guiStateScanning();
    int max         = ui->settings->batchConfig().max;
    numPendingScans = max <= 0 ? std::numeric_limits<int>::max() : max;
    scanFuture      = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanAbort() {
    numPendingScans = 0;
    saneDevice->cancelScan();
}

void ScanRoot::switchDevice() { mainWindow->showDeviceSelection(); }

void ScanRoot::updateSaneDevice(std::unique_ptr<lib::SaneDevice> device) {
    saneDevice = std::move(device);
    if (!saneDevice) {
        mainWindow->config().lastDevice = {};
        return;
    }

    ui->scanProgress->setEnabled(true);
    ui->scanProgress->setMinimum(0);
    ui->scanProgress->setMaximum(0);
    ui->scanProgress->setValue(-1);
    ui->mainTabs->setEnabled(false);
    ui->cfgRootWidget->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnScanOne->setEnabled(false);
    ui->btnScanBatch->setEnabled(false);

    connectFuture = std::async(std::launch::async, &ScanRoot::doConnect, this);
}

void ScanRoot::doConnect() {
    try {
        saneDevice->connect();
        if (mainWindow->config().lastDevice) {
            saneDevice->applyOptionSnapshot(mainWindow->config().lastDevice->options);
        }
    } catch (SaneException &ex) {
        lastException = ex;
        emit signalConnectionFailed();
        return;
    }

    emit signalConnected();
}

void ScanRoot::deviceConnected() {
    mainWindow->config().lastDevice = QScanConfig::LastDevice{
        saneDevice->getName(),
        saneDevice->getVendor(),
        saneDevice->getModel(),
        saneDevice->getType(),
        saneDevice->optionsSnapshot(),
    };

    ui->scanProgress->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnScanOne->setEnabled(true);
    ui->btnScanBatch->setEnabled(true);
    ui->mainTabs->setEnabled(true);
    ui->cfgRootWidget->setEnabled(true);

    // Initially populate options
    ui->settings->reloadOptions();
}

void ScanRoot::connectionFailed() {
    mainWindow->config().lastDevice = {};

    ui->scanProgress->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnScanOne->setEnabled(false);
    ui->btnScanBatch->setEnabled(false);
    ui->mainTabs->setEnabled(false);
    ui->cfgRootWidget->setEnabled(false);

    QMessageBox::critical(this,
                          tr("Failed to connect"),
                          tr("<b>Failed to connect to device:</b><br>%1<br><br><code>%2</code>")
                              .arg(QString::fromStdString(saneDevice->getName()))
                              .arg(lastException.what()));
    switchDevice();
}

void ScanRoot::newTab() {
    auto         *tab     = new ImagesTab(ui->mainTabs);
    const QString tabName = tr("Document #%1").arg(QString::number(++tabCounter));
    const int     tabIdx  = ui->mainTabs->addTab(tab, tabName);
    ui->mainTabs->setCurrentIndex(tabIdx);
}

void ScanRoot::closeTab(int _idx) {
    ImagesTab *tab = currentTab();
    if (tab && tab->areAnyImagesModified()) {
        QMessageBox::StandardButton res =
            QMessageBox::warning(this,
                                 tr("Unsaved images"),
                                 tr("Not all images have been saved. Do you still want to close the tab?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);

        if (res != QMessageBox::Yes) {
            return;
        }
    }
    ui->mainTabs->removeTab(_idx);
}

void ScanRoot::deviceOptionsReloaded() {
    QRect newPreviewRect = ui->settings->maxScanArea();
    if (!newPreviewRect.isNull() || newPreviewRect != previewSize) {
        QImage whitePreviewImg{newPreviewRect.width(), newPreviewRect.height(), QImage::Format_RGB32};
        whitePreviewImg.fill(Qt::white);
        ui->preview->updateImage(whitePreviewImg);

        previewSize = newPreviewRect;
    }

    QRect selection = ui->settings->currentScanArea();
    QRect max       = ui->settings->maxScanArea();
    if (selection == max) {
        selection = QRect();
    }
    ui->preview->updateSelection(selection);
}

void ScanRoot::updateProgressBar() {
    auto now = std::chrono::system_clock::now();
    if (delayScanUntil > now) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - delayScanStart);
        auto total   = std::chrono::duration_cast<std::chrono::milliseconds>(delayScanUntil - delayScanStart);
        ui->scanProgress->setMinimum(0);
        ui->scanProgress->setMaximum((int)total.count());
        ui->scanProgress->setValue((int)elapsed.count());
        ui->lScanInfo->setText(tr("Waiting:"));
        return;
    }

    double progress = saneDevice->scanProgress();
    if (progress <= 0.001) {
        ui->scanProgress->setMinimum(0);
        ui->scanProgress->setMaximum(0);
        ui->lScanInfo->setText(tr("Scan is starting:"));
        return;
    }
    ui->scanProgress->setMinimum(0);
    ui->scanProgress->setMaximum(1000);
    ui->scanProgress->setValue((int)(progress * 1000.0));
    ui->lScanInfo->setText(tr("Scanning:"));
}

ImagesTab *ScanRoot::currentTab() {
    if (ui->mainTabs->currentIndex() <= 0) {
        return nullptr;
    }

    return dynamic_cast<ImagesTab *>(ui->mainTabs->currentWidget());
}

bool ScanRoot::hasUnsavedImages() {
    for (int i = 1; i < ui->mainTabs->count(); ++i) {
        auto *tab = dynamic_cast<ImagesTab *>(ui->mainTabs->widget(i));
        if (!tab) {
            continue;
        }
        if (tab->areAnyImagesModified()) {
            return true;
        }
    }
    return false;
}

void ScanRoot::selectTabWithUnsavedImages() {
    for (int i = 1; i < ui->mainTabs->count(); ++i) {
        auto *tab = dynamic_cast<ImagesTab *>(ui->mainTabs->widget(i));
        if (!tab) {
            continue;
        }
        if (tab->areAnyImagesModified()) {
            ui->mainTabs->setCurrentIndex(i);
            return;
        }
    }
}

void ScanRoot::handleTabChanged(int) {
    ImagesTab *tab = currentTab();
    if (!tab) {
        mainWindow->setCanSave(false);
        return;
    }
    mainWindow->setCanSave(tab->currentImage() != nullptr);
}

void ScanRoot::doSave() {
    ImagesTab *tab = currentTab();
    if (!tab) {
        return;
    }
    tab->doSave();
}

void ScanRoot::doSaveAll() {
    ImagesTab *tab = currentTab();
    if (!tab) {
        return;
    }
    tab->doSaveAll();
}

void ScanRoot::setMainWindow(MainWindow *_mainWindow) {
    mainWindow = _mainWindow;
    ui->settings->setScanRoot(this);
}



} // namespace qscan::gui
