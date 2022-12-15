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
}

} // namespace qscan::gui
