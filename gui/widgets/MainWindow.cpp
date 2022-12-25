#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "ScanRoot.hpp"
#include "qscan_config.hpp"
#include "qscan_log.hpp"

#include <QCloseEvent>
#include <QMessageBox>

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

    restoreGeometry(cfg.window.geometry);
    restoreState(cfg.window.state);
}

MainWindow::~MainWindow() {
    cfg.window.geometry = saveGeometry();
    cfg.window.state    = saveState();
}

void MainWindow::showAbout() {
    const char *about = R"(
<b>Version: %1</b>
<br>
<br>
Authors:
<ul>
    <li>Daniel Mensinger (@mensinda)</li>
</ul>
)";

    QMessageBox::about(this, tr("About QScan"), tr(about).arg(QString::fromStdString(QSCAN_VERSION_STR)));
}
void MainWindow::showAboutQt() { QMessageBox::aboutQt(this); }

void MainWindow::showDeviceSelection() {
    ui->statusbar->showMessage(tr("Sane %1").arg(QString::fromStdString(backend.saneVersionStr())));
    ui->scanRoot->hide();
    ui->scanRoot->updateSaneDevice(nullptr);
    ui->deviceSelection->show();
    ui->deviceSelection->refreshDevices();
    setCanSave(false);
}

void MainWindow::deviceSelected(std::unique_ptr<lib::SaneDevice> device) {
    ui->statusbar->showMessage(tr("Sane %1 -- Device: %2")
                                   .arg(QString::fromStdString(backend.saneVersionStr()))
                                   .arg(QString::fromStdString(device->getName())));

    ui->deviceSelection->hide();
    ui->scanRoot->show();
    ui->scanRoot->updateSaneDevice(std::move(device));
}

bool MainWindow::doClose() {
    // Check if closeable
    if (ui->scanRoot->hasUnsavedImages() && !ui->scanRoot->isHidden()) {
        ui->scanRoot->selectTabWithUnsavedImages();
        QMessageBox::StandardButton btn =
            QMessageBox::warning(this,
                                 tr("Unsaved images"),
                                 tr("Not all images have been saved. Do you still want to quit?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
    }

    return close();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Check if closeable
    if (ui->scanRoot->hasUnsavedImages() && !ui->scanRoot->isHidden()) {
        ui->scanRoot->selectTabWithUnsavedImages();
        QMessageBox::StandardButton btn =
            QMessageBox::warning(this,
                                 tr("Unsaved images"),
                                 tr("Not all images have been saved. Do you still want to quit?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);
        if (btn == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    event->accept();
}


void MainWindow::setCanSave(bool _canSave) {
    ui->actionSave->setEnabled(_canSave);
    ui->actionSave_all->setEnabled(_canSave);
}

} // namespace qscan::gui
