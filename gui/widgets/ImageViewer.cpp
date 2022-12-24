#include "ImageViewer.hpp"
#include "ui_ImageViewer.h"

#include "qscan_log.hpp"

using namespace qscan::lib;

namespace qscan::gui {

ImageViewer::ImageViewer(QWidget *_parent) : QWidget(_parent), ui(new Ui::ImageViewer) {
    ui->setupUi(this);

    updateSelection(QRect{});
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

void ImageViewer::updateImage(const QImage &_img) { ui->viewer->updateImage(_img); }

ScanImageView *ImageViewer::imageView() { return ui->viewer; }

} // namespace qscan::gui
