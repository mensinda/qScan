#include "ImageViewer.hpp"
#include "qscan_log.hpp"
#include "ui_ImageViewer.h"

using namespace qscan::lib;

namespace qscan::gui {

ImageViewer::ImageViewer(QWidget *_parent) : QWidget(_parent), ui(new Ui::ImageViewer) {
    ui->setupUi(this);

    // todo
}

ImageViewer::~ImageViewer() {}

void ImageViewer::selectionChanged(QRect _selectionRect) {
    logger()->debug("Selection changed: {}x{} {}x{}",
                    _selectionRect.x(),
                    _selectionRect.y(),
                    _selectionRect.width(),
                    _selectionRect.height());
    emit selectionUpdated(_selectionRect);
}

void ImageViewer::updateSelection(QRect _newSelection) {
    logger()->warn("UPDATE TO: {}x{} {}x{}", _newSelection.x(), _newSelection.y(), _newSelection.width(), _newSelection.height());
    ui->viewer->updateSelection(_newSelection);
}

void ImageViewer::updateImage(QImage &_img) {
    ui->viewer->updateImage(_img);
}

} // namespace qscan::gui
