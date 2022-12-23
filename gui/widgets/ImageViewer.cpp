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
    ui->clearSelection->setEnabled(!_selectionRect.isNull());
    emit selectionUpdated(_selectionRect);
}

void ImageViewer::updateSelection(QRect _newSelection) {
    ui->viewer->updateSelection(_newSelection);
    ui->clearSelection->setEnabled(!_newSelection.isNull());
}

void ImageViewer::updateImage(QImage &_img) { ui->viewer->updateImage(_img); }

ScanImageView *ImageViewer::imageView() { return ui->viewer; }

} // namespace qscan::gui
