#include "ScanRoot.hpp"
#include "util/QImageContainer.hpp"
#include "MainWindow.hpp"
#include <QMessageBox>
#include <QTabBar>
#include <future>

#include "ImagesTab.hpp"
#include "qscan_log.hpp"
#include "ui_ScanRoot.h"


using namespace qscan::lib;

namespace qscan::gui {

ScanRoot::ScanRoot(QWidget *parent) : QWidget(parent), ui(new Ui::ScanRoot), progressTimer(parent) {
    ui->setupUi(this);

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->settings->setScanRoot(this);

    progressTimer.setInterval(250);

    QObject::connect(&progressTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    QObject::connect(this, SIGNAL(scanHasFinished()), this, SLOT(scanDone()));
    QObject::connect(this, SIGNAL(scanHasFailed()), this, SLOT(scanFailed()));
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
    try {
        SaneImage img = saneDevice->scan();
        auto      raw = img.asRGB8();

        // Fixup for Qt QImage::Format_RGB888 does not seem to work
        std::vector<uint32_t> rawBytes(raw.size());
        for (size_t i = 0; i < raw.size(); ++i) {
            rawBytes[i] = qRgb(raw[i].r, raw[i].g, raw[i].b);
        }

        QImage qtImage{
            const_cast<const uchar *>(reinterpret_cast<uchar *>(rawBytes.data())),
            (int)img.width(),
            (int)(rawBytes.size() / img.width()),
            QImage::Format_ARGB32,
        };

        emit scanHasFinished();
        return {std::move(rawBytes), std::move(qtImage)};
    } catch (const SaneException &e) {
        lastException = e;
        emit scanFailed();
    } catch (const std::exception &e) {
        lastException = SaneException(SANE_STATUS_INVAL, e.what());
        emit scanFailed();
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
}

void ScanRoot::scanDone() {
    auto *tab = currentTab();
    if (tab == nullptr) {
        auto [_, qtImage] = scanFuture.get();
        ui->preview->updateImage(qtImage);
    } else {
        tab->addImage(scanFuture.get());
    }

    if (!optionsSnapshot.empty()) {
        saneDevice->applyOptionSnapshot(optionsSnapshot);
        optionsSnapshot.clear();
    }

    ui->settings->reloadOptions();
    guiStateReady();
}

void ScanRoot::scanFailed() {
    guiStateReady();
    if (!optionsSnapshot.empty()) {
        saneDevice->applyOptionSnapshot(optionsSnapshot);
        optionsSnapshot.clear();
    }

    QMessageBox::critical(this,
                          tr("Scanning failed"),
                          tr("<b>Failed to scan:</b><br>%1<br><br><code>%2</code>")
                              .arg(QString::fromStdString(saneDevice->getName()))
                              .arg(lastException.what()));
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
    scanFuture = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanOne() {
    // New tab when on preview
    if (ui->mainTabs->currentIndex() == 0) {
        newTab();
    }
    guiStateScanning();
    scanFuture = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanBatch() {}
void ScanRoot::scanAbort() {}

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
    QRect max = ui->settings->maxScanArea();
    if (selection == max) {
        selection = QRect();
    }
    ui->preview->updateSelection(selection);
}

void ScanRoot::updateProgressBar() {
    double progress = saneDevice->scanProgress();
    if (progress <= 0.001) {
        ui->scanProgress->setMinimum(0);
        ui->scanProgress->setMaximum(0);
        return;
    }
    ui->scanProgress->setMinimum(0);
    ui->scanProgress->setMaximum(1000);
    ui->scanProgress->setValue((int)(progress * 1000.0));
}

ImagesTab *ScanRoot::currentTab() {
    if (ui->mainTabs->currentIndex() <= 0) {
        return nullptr;
    }

    return dynamic_cast<ImagesTab *>(ui->mainTabs->currentWidget());
}


} // namespace qscan::gui
