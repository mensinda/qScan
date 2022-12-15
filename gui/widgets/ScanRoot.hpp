#pragma once

#include "SaneDevice.hpp"
#include "SaneException.hpp"
#include <QToolButton>
#include <QWidget>
#include <future>

namespace Ui {
class ScanRoot;
}

namespace qscan::gui {

class MainWindow;

class ScanRoot : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::ScanRoot>    ui;
    std::unique_ptr<lib::SaneDevice> saneDevice;

    MainWindow *mainWindow = nullptr;

    std::future<void>  connectFuture;
    lib::SaneException lastException{SANE_STATUS_GOOD, ""};

  public:
    explicit ScanRoot(QWidget *parent);
    virtual ~ScanRoot();

    void setMainWindow(MainWindow *_mainWindow) { mainWindow = _mainWindow; }

    [[nodiscard]] MainWindow      *getMainWindow() const { return mainWindow; }
    [[nodiscard]] lib::SaneDevice &getSaneDevice() { return *saneDevice; }

    void updateSaneDevice(std::unique_ptr<lib::SaneDevice> device);

  public slots:
    void scanOne();
    void scanBatch();
    void scanAbort();
    void switchDevice();

    void deviceConnected();
    void connectionFailed();

    void newTab();

  signals:
    void signalConnected();
    void signalConnectionFailed();

  private:
    void doConnect();
};

} // namespace qscan::gui
