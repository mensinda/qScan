#pragma once

#include "util/QImageContainer.hpp"
#include "ImagesTab.hpp"
#include "SaneDevice.hpp"
#include "SaneException.hpp"

#include <chrono>
#include <future>

#include <QTimer>
#include <QToolButton>
#include <QWidget>

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

    std::future<void>            connectFuture;
    std::future<QImageContainer> scanFuture;
    lib::SaneException           lastException{SANE_STATUS_GOOD, ""};
    lib::SaneDevice::snapshot_t  optionsSnapshot;

    std::chrono::time_point<std::chrono::system_clock> delayScanUntil;
    std::chrono::time_point<std::chrono::system_clock> delayScanStart;

    QRect  previewSize{};
    QTimer progressTimer;
    int    tabCounter      = 0;
    int    numPendingScans = 0;

  public:
    explicit ScanRoot(QWidget *parent);
    virtual ~ScanRoot();

    void setMainWindow(MainWindow *_mainWindow);

    [[nodiscard]] MainWindow      *getMainWindow() const { return mainWindow; }
    [[nodiscard]] lib::SaneDevice &getSaneDevice() { return *saneDevice; }

    [[nodiscard]] ImagesTab *currentTab();
    [[nodiscard]] bool       hasUnsavedImages();

    void selectTabWithUnsavedImages();

    void updateSaneDevice(std::unique_ptr<lib::SaneDevice> device);

  public slots:
    void scanPreview();
    void scanOne();
    void scanBatch();
    void scanAbort();
    void handleScanDone();
    void handleScanFailed();
    void switchDevice();

    void deviceConnected();
    void connectionFailed();

    void newTab();
    void closeTab(int _idx);
    void handleTabChanged(int _idx);
    void deviceOptionsReloaded();
    void updateProgressBar();

    void doSave();
    void doSaveAll();

  signals:
    void signalConnected();
    void signalConnectionFailed();
    void scanHasFinished();
    void scanHasFailed();

  private:
    void            doConnect();
    QImageContainer doScan();

    void guiStateReady();
    void guiStateScanning();
};

} // namespace qscan::gui
