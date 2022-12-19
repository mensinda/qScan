#include "ScanRoot.hpp"
#include "MainWindow.hpp"
#include <QMessageBox>
#include <QTabBar>
#include <future>

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

ScanRoot::QImageContainer ScanRoot::doScan() {
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
}

void ScanRoot::scanDone() {
    auto [_, qtImage] = scanFuture.get();
    ui->preview->updateImage(qtImage);

    progressTimer.stop();

    ui->scanProgress->setEnabled(false);
    ui->scanProgress->setValue(0);
    ui->cfgRootWidget->setEnabled(true);
    ui->btnPreview->setEnabled(true);
    ui->btnScanOne->setEnabled(true);
    ui->btnScanBatch->setEnabled(true);
    ui->btnStop->setEnabled(false);
}

void ScanRoot::scanPreview() {
    saneDevice->getOptions().setPreview(false);
    progressTimer.start();
    ui->cfgRootWidget->setEnabled(false);
    ui->btnPreview->setEnabled(false);
    ui->btnScanOne->setEnabled(false);
    ui->btnScanBatch->setEnabled(false);
    ui->btnStop->setEnabled(true);
    scanFuture = std::async(std::launch::async, &ScanRoot::doScan, this);
}

void ScanRoot::scanOne() {}
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
    // TODO
    logger()->info("New tab!");
}

void ScanRoot::deviceOptionsReloaded() {
    QRect newPreviewRect = ui->settings->maxScanArea();
    if (newPreviewRect.isNull() || newPreviewRect == previewSize) {
        return;
    }

    QImage whitePreviewImg{newPreviewRect.width(), newPreviewRect.height(), QImage::Format_RGB32};
    whitePreviewImg.fill(Qt::white);
    ui->preview->updateImage(whitePreviewImg);

    previewSize = newPreviewRect;
}

void ScanRoot::updateProgressBar() {
    ui->scanProgress->setEnabled(true);
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


} // namespace qscan::gui
