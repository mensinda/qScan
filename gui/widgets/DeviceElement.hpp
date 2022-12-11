#pragma once

#include "SaneBackend.hpp"
#include <QWidget>
#include <future>
#include <memory>

namespace Ui {
class DeviceElement;
}

namespace qscan::gui {

class DeviceSelection;

class DeviceElement : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::DeviceElement> ui;
    std::unique_ptr<lib::SaneDevice> saneDevice;

    DeviceSelection *selectionWidget;

  private:
    void doBackendReload();

  public:
    explicit DeviceElement(DeviceSelection *_selectionWidget, std::unique_ptr<lib::SaneDevice> _saneDevice);
    virtual ~DeviceElement();

  public slots:
    void thisElementSelected();
};

}
