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

void ImageViewer::selectionChanged() {
    logger()->debug("Selection changed");
}

}
