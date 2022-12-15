#include "OptionsWidget.hpp"
#include "MainWindow.hpp"
#include "ScanRoot.hpp"
#include "qscan_log.hpp"
#include "ui_OptionsWidget.h"

using namespace qscan::lib;

namespace qscan::gui {

OptionsWidget::OptionsWidget(QWidget *_parent) : QWidget(_parent), ui(new Ui::OptionsWidget) { ui->setupUi(this); }

OptionsWidget::~OptionsWidget() {}

void OptionsWidget::reloadOptions() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();

    auto sourceOpt     = opts.getSource();
    auto resolutionOpt = opts.getResolution();
    auto modeOpt       = opts.getMode();
    auto scanAreaOpt   = opts.getScanArea();

    (void)resolutionOpt;
    (void)modeOpt;
    (void)scanAreaOpt;

    // Source
    ui->coSource->blockSignals(true);
    ui->coSource->clear();
    ui->coSource->setEnabled((bool)sourceOpt);
    if (sourceOpt) {
        for (const auto &i : sourceOpt->values) {
            ui->coSource->addItem(QString::fromStdString(i));
        }
        ui->coSource->setCurrentText(QString::fromStdString(sourceOpt->current));
        ui->coSource->setToolTip(QString::fromStdString(sourceOpt->description));
        ui->lSource->setToolTip(QString::fromStdString(sourceOpt->description));
    }
    ui->coSource->blockSignals(false);

    // Mode
    ui->coMode->blockSignals(true);
    ui->coMode->clear();
    ui->coMode->setEnabled((bool)modeOpt);
    if (modeOpt) {
        for (const auto &i : modeOpt->values) {
            ui->coMode->addItem(QString::fromStdString(i));
        }
        ui->coMode->setCurrentText(QString::fromStdString(modeOpt->current));
        ui->coMode->setToolTip(QString::fromStdString(modeOpt->description));
        ui->lMode->setToolTip(QString::fromStdString(modeOpt->description));
    }
    ui->coMode->blockSignals(false);

    // Resolution
    ui->coResolution->blockSignals(true);
    ui->coResolution->clear();
    ui->coResolution->setEnabled((bool)modeOpt);
    if (modeOpt) {
        for (const double i : resolutionOpt->values) {
            auto displayStr = fmt::format("{} {}", (int)i, SaneBackend::saneUnitToDisplayString(resolutionOpt->unit));
            ui->coResolution->addItem(QString::fromStdString(displayStr), QVariant(i));
        }
        ui->coResolution->setCurrentText(QString::number((int)resolutionOpt->current));
        ui->coResolution->setToolTip(QString::fromStdString(resolutionOpt->description));
        ui->lResolution->setToolTip(QString::fromStdString(resolutionOpt->description));
    }
    ui->coResolution->blockSignals(false);
}

void OptionsWidget::sourceUpdated() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();
    if (opts.setSource(ui->coSource->currentText().toStdString())) {
        reloadOptions();
    }
}

void OptionsWidget::modeUpdated() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();
    if (opts.setMode(ui->coMode->currentText().toStdString())) {
        reloadOptions();
    }
}

void OptionsWidget::resolutionUpdated() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();
    if (opts.setResolution(ui->coResolution->currentData().toDouble())) {
        reloadOptions();
    }
}

void OptionsWidget::scanAreaUpdated() {}

void OptionsWidget::batchSettingsUpdated() {}

MainWindow *OptionsWidget::getMainWindow() const { return scanRoot->getMainWindow(); }


} // namespace qscan::gui
