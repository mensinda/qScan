#include "OptionsWidget.hpp"
#include "MainWindow.hpp"
#include "ScanRoot.hpp"
#include "enum2str.hpp"
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

    emit optionsReloaded();

    QSignalBlocker blocker01{ui->coSource};
    QSignalBlocker blocker02{ui->coMode};
    QSignalBlocker blocker03{ui->coResolution};
    QSignalBlocker blocker04{ui->topLeftX};
    QSignalBlocker blocker05{ui->topLeftY};
    QSignalBlocker blocker06{ui->bottomRightX};
    QSignalBlocker blocker07{ui->bottomRightY};


    // Source
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

    // Mode
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

    // Resolution
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

    // Scan area
    ui->topLeftX->setEnabled((bool)scanAreaOpt);
    ui->topLeftY->setEnabled((bool)scanAreaOpt);
    ui->bottomRightX->setEnabled((bool)scanAreaOpt);
    ui->bottomRightY->setEnabled((bool)scanAreaOpt);
    if (scanAreaOpt) {
        ui->topLeftX->setMinimum(scanAreaOpt->min.topLeftX);
        ui->topLeftY->setMinimum(scanAreaOpt->min.topLeftY);
        ui->bottomRightX->setMinimum(scanAreaOpt->current.topLeftX);
        ui->bottomRightY->setMinimum(scanAreaOpt->current.topLeftY);

        ui->topLeftX->setMaximum(scanAreaOpt->current.bottomRightX);
        ui->topLeftY->setMaximum(scanAreaOpt->current.bottomRightY);
        ui->bottomRightX->setMaximum(scanAreaOpt->max.bottomRightX);
        ui->bottomRightY->setMaximum(scanAreaOpt->max.bottomRightY);

        ui->topLeftX->setValue(scanAreaOpt->current.topLeftX);
        ui->topLeftY->setValue(scanAreaOpt->current.topLeftY);
        ui->bottomRightX->setValue(scanAreaOpt->current.bottomRightX);
        ui->bottomRightY->setValue(scanAreaOpt->current.bottomRightY);

        const QString suffix = QString::fromStdString(" " + SaneBackend::saneUnitToDisplayString(scanAreaOpt->unit));
        ui->topLeftX->setSuffix(suffix);
        ui->topLeftY->setSuffix(suffix);
        ui->bottomRightX->setSuffix(suffix);
        ui->bottomRightY->setSuffix(suffix);
    }
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

void OptionsWidget::scanAreaUpdated() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();

    const SaneOptionsWrapper::ScanArea area{
        ui->bottomRightX->value(),
        ui->bottomRightY->value(),
        ui->topLeftX->value(),
        ui->topLeftY->value(),
    };

    if (opts.setScanArea(area)) {
        reloadOptions();
    }

    auto scanAreaOpt = opts.getScanArea();
    if (!scanAreaOpt) {
        return;
    }

    const double dpmm = getDpmm();

    switch (scanAreaOpt->unit) {
        case SANE_UNIT_PIXEL:
            emit scanAreaHasUpdated({
                QPoint{(int)ui->topLeftX->value(),     (int)ui->topLeftY->value()    },
                QPoint{(int)ui->bottomRightX->value(), (int)ui->bottomRightY->value()},
            });
            break;
        case SANE_UNIT_MM:
            emit scanAreaHasUpdated({
                QPoint{(int)(ui->topLeftX->value() * dpmm),     (int)(ui->topLeftY->value() * dpmm)    },
                QPoint{(int)(ui->bottomRightX->value() * dpmm), (int)(ui->bottomRightY->value() * dpmm)},
            });
            break;
        default: throw std::runtime_error("Unsupported unit: " + enum2str::toStr(scanAreaOpt->unit));
    }
}

void OptionsWidget::batchSettingsUpdated() {}

MainWindow *OptionsWidget::getMainWindow() const { return scanRoot->getMainWindow(); }

void OptionsWidget::updateScanArea(QRect _rect) {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();

    auto scanAreaOpt = opts.getScanArea();
    if (!scanAreaOpt) {
        return;
    }

    QSignalBlocker blocker04{ui->topLeftX};
    QSignalBlocker blocker05{ui->topLeftY};
    QSignalBlocker blocker06{ui->bottomRightX};
    QSignalBlocker blocker07{ui->bottomRightY};

    if (_rect.isNull()) {
        ui->topLeftX->setValue(scanAreaOpt->min.topLeftX);
        ui->topLeftY->setValue(scanAreaOpt->min.topLeftY);
        ui->bottomRightX->setValue(scanAreaOpt->max.bottomRightX);
        ui->bottomRightY->setValue(scanAreaOpt->max.bottomRightY);
    } else {
        const double dpmm = getDpmm();

        switch (scanAreaOpt->unit) {
            case SANE_UNIT_PIXEL:
                ui->topLeftX->setValue(_rect.topLeft().x());
                ui->topLeftY->setValue(_rect.topLeft().y());
                ui->bottomRightX->setValue(_rect.bottomRight().x());
                ui->bottomRightY->setValue(_rect.bottomRight().y());
                break;
            case SANE_UNIT_MM:
                ui->topLeftX->setValue((double)_rect.topLeft().x() / dpmm);
                ui->topLeftY->setValue((double)_rect.topLeft().y() / dpmm);
                ui->bottomRightX->setValue((double)_rect.bottomRight().x() / dpmm);
                ui->bottomRightY->setValue((double)_rect.bottomRight().y() / dpmm);
                break;
            default: throw std::runtime_error("Unsupported unit: " + enum2str::toStr(scanAreaOpt->unit));
        }
    }

    const SaneOptionsWrapper::ScanArea area{
        ui->bottomRightX->value(),
        ui->bottomRightY->value(),
        ui->topLeftX->value(),
        ui->topLeftY->value(),
    };

    if (opts.setScanArea(area)) {
        reloadOptions();
    }
}

QRect OptionsWidget::maxScanArea() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();

    const auto &scanAreaOpt = opts.getScanArea();
    if (!scanAreaOpt) {
        return {};
    }

    const auto &maxA = scanAreaOpt->max;
    const auto &minA = scanAreaOpt->min;

    const double dpmm = getDpmm();

    switch (scanAreaOpt->unit) {
        case SANE_UNIT_PIXEL:
            return {
                (int)minA.topLeftX,
                (int)minA.topLeftY,
                (int)maxA.bottomRightX,
                (int)maxA.bottomRightY,
            };
        case SANE_UNIT_MM:
            return {
                (int)(minA.topLeftX * dpmm),
                (int)(minA.topLeftY * dpmm),
                (int)(maxA.bottomRightX * dpmm),
                (int)(maxA.bottomRightY * dpmm),
            };
        default: throw std::runtime_error("Unsupported unit: " + enum2str::toStr(scanAreaOpt->unit));
    }
}

double OptionsWidget::getDpmm() {
    SaneOptionsWrapper &opts          = scanRoot->getSaneDevice().getOptions();
    const auto         &resolutionOpt = opts.getResolution();
    if (!resolutionOpt) {
        return 1;
    }

    return resolutionOpt->current / 25.4;
}


} // namespace qscan::gui
