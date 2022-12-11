#include "MainWindow.hpp"
#include "ScanRoot.hpp"

#include "qscan_config.hpp"
#include "qscan_log.hpp"

#include <QMessageBox>

#include "ui_MainWindow.h"

using namespace qscan::lib;

namespace qscan::gui {

MainWindow::MainWindow(QWidget *parent, SaneBackend &_backend, QScanConfig &_cfg)
    : QMainWindow(parent), backend(_backend), cfg(_cfg), ui(new Ui::MainWindow) {
    // Only ui
    ui->setupUi(this);
    ui->deviceSelection->setMainWindow(this);
    ui->scanRoot->setMainWindow(this);

    if (cfg.lastDevice) {
        SANE_Device dev{
            cfg.lastDevice->name.c_str(),
            cfg.lastDevice->vendor.c_str(),
            cfg.lastDevice->model.c_str(),
            cfg.lastDevice->type.c_str(),
        };
        deviceSelected(std::make_unique<lib::SaneDevice>(&dev));
    } else {
        showDeviceSelection();
    }
}

MainWindow::~MainWindow() {}

void MainWindow::showAbout() {
    QMessageBox::about(this, tr("About QScan"), tr("Version: %1").arg(QString::fromStdString(QSCAN_VERSION_STR)));
}
void MainWindow::showAboutQt() { QMessageBox::aboutQt(this); }

void MainWindow::showDeviceSelection() {
    ui->statusbar->showMessage(tr("Sane %1").arg(QString::fromStdString(backend.saneVersionStr())));
    ui->scanRoot->hide();
    ui->scanRoot->updateSaneDevice(nullptr);
    ui->deviceSelection->show();
    ui->deviceSelection->refreshDevices();
}

void MainWindow::deviceSelected(std::unique_ptr<lib::SaneDevice> device) {
    ui->statusbar->showMessage(tr("Sane %1 -- Device: %2")
                                   .arg(QString::fromStdString(backend.saneVersionStr()))
                                   .arg(QString::fromStdString(device->getName())));

    ui->deviceSelection->hide();
    ui->scanRoot->show();
    ui->scanRoot->updateSaneDevice(std::move(device));
}

} // namespace qscan::gui
