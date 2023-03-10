#include "DeviceSelection.hpp"
#include "ui_DeviceSelection.h"

#include "DeviceElement.hpp"
#include "MainWindow.hpp"
#include "SaneException.hpp"
#include "qscan_log.hpp"

#include <future>

#include <QInputDialog>

using namespace qscan::lib;

namespace qscan::gui {

DeviceSelection::DeviceSelection(QWidget *parent) : QWidget(parent), ui(new Ui::DeviceSelection) { ui->setupUi(this); }

DeviceSelection::~DeviceSelection() {
    if (scanFuture.valid()) {
        scanFuture.get();
    };
}

void DeviceSelection::refreshDevices() {
    elements.clear();
    ui->scanLabel->show();
    ui->selectLabel->hide();
    ui->avahiNoteLabel->hide();
    ui->progressBar->setEnabled(true);
    ui->manualBtn->setEnabled(false);
    ui->refreshBtn->setEnabled(false);
    ui->scrollArea->setEnabled(false);

    scanFuture = std::async(std::launch::async, &DeviceSelection::doBackendReload, this);
}

void DeviceSelection::devicesLoaded() {
    ui->scanLabel->hide();
    ui->selectLabel->show();
    ui->avahiNoteLabel->show();
    ui->progressBar->setEnabled(false);
    ui->manualBtn->setEnabled(true);
    ui->refreshBtn->setEnabled(true);
    ui->scrollArea->setEnabled(true);

    for (auto &saneDevice : saneDevices) {
        elements.emplace_back(new DeviceElement(this, std::move(saneDevice)));
        ui->scrollLayout->insertWidget(0, elements.back().get());
    }
}

void DeviceSelection::doBackendReload() {
    saneDevices = mainWindow->getBackend().find_devices();
    emit finishedReloading();
}

void DeviceSelection::deviceSelected(std::unique_ptr<lib::SaneDevice> device) {
    mainWindow->deviceSelected(std::move(device));
}

void DeviceSelection::tryConnectManually() {
    bool isOk;

    QString url = QInputDialog::getText(this,
                                        tr("Manual device connection"),
                                        tr("Please enter the scanner device URL here:"),
                                        QLineEdit::Normal,
                                        "",
                                        &isOk);
    if (!isOk) {
        return;
    }

    mainWindow->deviceSelected(std::make_unique<lib::SaneDevice>(url.toStdString()));
}

} // namespace qscan::gui
