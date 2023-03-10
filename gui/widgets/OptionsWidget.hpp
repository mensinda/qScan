#pragma once

#include "config/QScanConfig.hpp"

#include <memory>

#include <QWidget>

namespace Ui {
class OptionsWidget;
}

namespace qscan::gui {

class ScanRoot;
class MainWindow;

class OptionsWidget : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::OptionsWidget> ui;

    ScanRoot *scanRoot = nullptr;

  public:
    explicit OptionsWidget(QWidget *_parent);
    virtual ~OptionsWidget();

    void setScanRoot(ScanRoot *_scanRoot);

    [[nodiscard]] ScanRoot   *getScanRoot() const { return scanRoot; }
    [[nodiscard]] MainWindow *getMainWindow() const;

    [[nodiscard]] QRect  maxScanArea();
    [[nodiscard]] QRect  currentScanArea();
    [[nodiscard]] double getDpmm();

    [[nodiscard]] QScanConfig::BatchScanning batchConfig();

  public slots:
    void sourceUpdated();
    void modeUpdated();
    void resolutionUpdated();
    void batchSettingsUpdated();
    void scanAreaUpdated();

    void reloadOptions();

    void updateScanArea(QRect _rect);

  signals:
    void scanAreaHasUpdated(QRect _newArea);
    void optionsReloaded();
};

} // namespace qscan::gui
