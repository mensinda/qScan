#pragma once

#include <QWidget>
#include <memory>

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

    void setScanRoot(ScanRoot *_scanRoot) { scanRoot = _scanRoot; }

    [[nodiscard]] ScanRoot   *getScanRoot() const { return scanRoot; }
    [[nodiscard]] MainWindow *getMainWindow() const;

    [[nodiscard]] QRect maxScanArea();
    [[nodiscard]] QRect currentScanArea();
    [[nodiscard]] double getDpmm();

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
