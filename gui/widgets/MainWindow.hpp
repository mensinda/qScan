#pragma once

#include "config/QScanConfig.hpp"

#include <SaneBackend.hpp>
#include <memory>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace qscan::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

  private:
    lib::SaneBackend &backend;
    QScanConfig      &cfg;

  public:
    explicit MainWindow(QWidget *parent, lib::SaneBackend &_backend, QScanConfig &_cfg);
    virtual ~MainWindow();

    [[nodiscard]] lib::SaneBackend &getBackend() { return backend; }
    [[nodiscard]] QScanConfig      &config() { return cfg; }

    void showDeviceSelection();
    void deviceSelected(std::unique_ptr<lib::SaneDevice> device);

    void setCanSave(bool _canSave);

  public slots:
    void showAbout();
    void showAboutQt();
    bool doClose();

  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    std::unique_ptr<Ui::MainWindow> ui;
};

} // namespace qscan::gui
