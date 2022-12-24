#include "OptionsWidget.hpp"
#include "MainWindow.hpp"
#include "ScanRoot.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"
#include "ui_OptionsWidget.h"

using namespace qscan::lib;

namespace qscan::gui {

OptionsWidget::OptionsWidget(QWidget *_parent) : QWidget(_parent), ui(new Ui::OptionsWidget) {
    ui->setupUi(this);
}

OptionsWidget::~OptionsWidget() { scanRoot->getMainWindow()->config().batch = batchConfig(); }

void OptionsWidget::reloadOptions() {
    SaneOptionsWrapper &opts = scanRoot->getSaneDevice().getOptions();

    auto sourceOpt     = opts.getSource();
    auto resolutionOpt = opts.getResolution();
    auto modeOpt       = opts.getMode();
    auto scanAreaOpt   = opts.getScanArea();

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
        const std::string unitStr = SaneBackend::saneUnitToDisplayString(resolutionOpt->unit);
        for (const double i : resolutionOpt->values) {
            auto displayStr = fmt::format("{} {}", (int)i, unitStr);
            ui->coResolution->addItem(QString::fromStdString(displayStr), QVariant(i));
        }
        auto displayStr = fmt::format("{} {}", (int)resolutionOpt->current, unitStr);
        ui->coResolution->setCurrentText(QString::fromStdString(displayStr));
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

    emit optionsReloaded();
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
        ui->topLeftX->value(),
        ui->topLeftY->value(),
        ui->bottomRightX->value(),
        ui->bottomRightY->value(),
    };

    if (opts.setScanArea(area)) {
        reloadOptions();
    }

    auto scanAreaOpt = opts.getScanArea();
    if (!scanAreaOpt) {
        return;
    }

    QRect scanAreaRect = currentScanArea();

    // No need for a selection if it is the max anyway
    if (scanAreaRect == maxScanArea()) {
        scanAreaRect = QRect();
    }

    emit scanAreaHasUpdated(scanAreaRect);
}

void OptionsWidget::batchSettingsUpdated() {
    ui->batchMax->setEnabled(ui->batchMaxCB->checkState() == Qt::Checked);
    ui->batchDelay->setEnabled(ui->batchDelayCB->checkState() == Qt::Checked);
    ui->batchDelayS->setEnabled(ui->batchDelayCB->checkState() == Qt::Checked);
}

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
        ui->topLeftX->value(),
        ui->topLeftY->value(),
        ui->bottomRightX->value(),
        ui->bottomRightY->value(),
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

QRect OptionsWidget::currentScanArea() {
    SaneOptionsWrapper &opts        = scanRoot->getSaneDevice().getOptions();
    const auto         &scanAreaOpt = opts.getScanArea();
    if (!scanAreaOpt) {
        return {};
    }

    const auto &maxA = scanAreaOpt->max;
    const auto &minA = scanAreaOpt->min;
    const auto &curA = scanAreaOpt->current;

    // If we are close to the maximal scan area, just use
    // the `maxScanArea` to avoid rounding issues.
    const double epsilon = 0.01;
    if ((std::abs(curA.topLeftX - minA.topLeftX) < epsilon) && (std::abs(curA.topLeftY - minA.topLeftY) < epsilon) &&
        (std::abs(curA.bottomRightX - maxA.bottomRightX) < epsilon) &&
        (std::abs(curA.bottomRightY - maxA.bottomRightY) < epsilon)) {
        return maxScanArea();
    }

    const double dpmm = getDpmm();

    switch (scanAreaOpt->unit) {
        case SANE_UNIT_PIXEL:
            return {
                QPoint{(int)curA.topLeftX,     (int)curA.topLeftY    },
                QPoint{(int)curA.bottomRightX, (int)curA.bottomRightY},
            };
        case SANE_UNIT_MM:
            return {
                QPoint{(int)(curA.topLeftX * dpmm),     (int)(curA.topLeftY * dpmm)    },
                QPoint{(int)(curA.bottomRightX * dpmm), (int)(curA.bottomRightY * dpmm)},
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

QScanConfig::BatchScanning OptionsWidget::batchConfig() {
    int max   = ui->batchMax->value();
    int delay = ui->batchDelay->value();
    if (ui->batchMaxCB->checkState() == Qt::Unchecked) {
        max = -max - 1;
    }
    if (ui->batchDelayCB->checkState() == Qt::Unchecked) {
        delay = -delay - 1;
    }
    return {max, delay};
}

void OptionsWidget::setScanRoot(ScanRoot *_scanRoot) {
    scanRoot = _scanRoot;

    auto [max, delay] = scanRoot->getMainWindow()->config().batch;
    ui->batchMaxCB->setCheckState(max < 0 ? Qt::Unchecked : Qt::Checked);
    ui->batchDelayCB->setCheckState(delay < 0 ? Qt::Unchecked : Qt::Checked);
    if (max < 0) {
        max = -max - 1;
    }
    if (delay < 0) {
        delay = -delay - 1;
    }
    ui->batchMax->setValue(max);
    ui->batchDelay->setValue(delay);
}


} // namespace qscan::gui
