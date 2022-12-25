#pragma once

#include "SaneBackend.hpp"

#include <future>
#include <memory>

#include <QVBoxLayout>
#include <QWidget>

namespace Ui {
class DeviceSelection;
}

namespace qscan::gui {

class MainWindow;
class DeviceElement;

class DeviceSelection : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::DeviceSelection> ui;

    MainWindow                  *mainWindow = nullptr;
    std::future<void>            scanFuture = {};
    lib::SaneBackend::DeviceList saneDevices;

    std::vector<std::unique_ptr<DeviceElement>> elements;

  private:
    void doBackendReload();

  public:
    explicit DeviceSelection(QWidget *parent);
    virtual ~DeviceSelection();

    [[nodiscard]] MainWindow *getMainWindow() const { return mainWindow; }

    void setMainWindow(MainWindow *_mainWindow) { mainWindow = _mainWindow; }
    void deviceSelected(std::unique_ptr<lib::SaneDevice> device);

  public slots:
    void refreshDevices();
    void devicesLoaded();
    void tryConnectManually();

  signals:
    void finishedReloading();
};

} // namespace qscan::gui
