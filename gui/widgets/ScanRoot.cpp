#include "ScanRoot.hpp"
#include "MainWindow.hpp"
#include <QMessageBox>
#include <QTabBar>
#include <future>

#include "qscan_log.hpp"
#include "ui_ScanRoot.h"


using namespace qscan::lib;

namespace qscan::gui {

ScanRoot::ScanRoot(QWidget *parent) : QWidget(parent), ui(new Ui::ScanRoot) {
    ui->setupUi(this);

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->settings->setScanRoot(this);

    QObject::connect(this, SIGNAL(signalConnected()), this, SLOT(deviceConnected()));
    QObject::connect(this, SIGNAL(signalConnectionFailed()), this, SLOT(connectionFailed()));
}

ScanRoot::~ScanRoot() {
    if (connectFuture.valid()) { connectFuture.get(); }
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

} // namespace qscan::gui
