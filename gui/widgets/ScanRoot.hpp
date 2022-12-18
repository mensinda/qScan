#pragma once

#include "SaneDevice.hpp"
#include "SaneException.hpp"
#include <QTimer>
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

  public:
    struct QImageContainer {
        std::vector<uint32_t> raw;
        QImage                img;
    };

  private:
    std::unique_ptr<Ui::ScanRoot>    ui;
    std::unique_ptr<lib::SaneDevice> saneDevice;

    MainWindow *mainWindow = nullptr;

    std::future<void>            connectFuture;
    std::future<QImageContainer> scanFuture;
    lib::SaneException           lastException{SANE_STATUS_GOOD, ""};

    QRect  previewSize{};
    QTimer progressTimer;

  public:
    explicit ScanRoot(QWidget *parent);
    virtual ~ScanRoot();

    void setMainWindow(MainWindow *_mainWindow) { mainWindow = _mainWindow; }

    [[nodiscard]] MainWindow      *getMainWindow() const { return mainWindow; }
    [[nodiscard]] lib::SaneDevice &getSaneDevice() { return *saneDevice; }

    void updateSaneDevice(std::unique_ptr<lib::SaneDevice> device);

  public slots:
    void scanPreview();
    void scanOne();
    void scanBatch();
    void scanAbort();
    void scanDone();
    void switchDevice();

    void deviceConnected();
    void connectionFailed();

    void newTab();
    void deviceOptionsReloaded();
    void updateProgressBar();

  signals:
    void signalConnected();
    void signalConnectionFailed();
    void scanHasFinished();

  private:
    void            doConnect();
    QImageContainer doScan();
};

} // namespace qscan::gui
