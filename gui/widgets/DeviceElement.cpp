#include "DeviceElement.hpp"
#include "ui_DeviceElement.h"

#include "DeviceSelection.hpp"
#include "qscan_log.hpp"

namespace qscan::gui {

DeviceElement::DeviceElement(DeviceSelection *_selectionWidget, std::unique_ptr<lib::SaneDevice> _saneDevice)
    : QWidget(nullptr),
      ui(new Ui::DeviceElement),
      saneDevice(std::move(_saneDevice)),
      selectionWidget(_selectionWidget) {
    ui->setupUi(this);
    ui->id->setText(QString::fromStdString(saneDevice->getName()));
    ui->vendor->setText(QString::fromStdString(saneDevice->getVendor()));
    ui->model->setText(QString::fromStdString(saneDevice->getModel()));
    ui->type->setText(QString::fromStdString(saneDevice->getType()));
}

DeviceElement::~DeviceElement() {}

void DeviceElement::thisElementSelected() { selectionWidget->deviceSelected(std::move(saneDevice)); }

} // namespace qscan::gui
