#include "OptionsWidget.hpp"
#include "MainWindow.hpp"
#include "ScanRoot.hpp"
#include "ui_OptionsWidget.h"

namespace qscan::gui {

OptionsWidget::OptionsWidget(QWidget *_parent) : QWidget(_parent), ui(new Ui::OptionsWidget) {
    ui->setupUi(this);
    // todo
}

OptionsWidget::~OptionsWidget() {}

void OptionsWidget::sourceUpdated() {}
void OptionsWidget::modeUpdated() {}
void OptionsWidget::resolutionUpdated() {}
void OptionsWidget::scanAreaUpdated() {}

void OptionsWidget::batchSettingsUpdated() {}

MainWindow *OptionsWidget::getMainWindow() const { return scanRoot->getMainWindow(); }


} // namespace qscan::gui
