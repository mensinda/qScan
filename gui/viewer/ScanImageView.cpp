#include "ScanImageView.hpp"
#include "qscan_log.hpp"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

using namespace qscan::lib;

namespace qscan::gui {

ScanImageView::ScanImageView(QWidget *parent) : QGraphicsView(parent) {
    overlay = std::make_unique<SelectionOverlay>(QRect{0, 0, 210, 297});
    overlay->setZValue(1);

    scanScene = std::make_unique<QGraphicsScene>(this);
    scanScene->addItem(overlay.get());

    QImage img{210, 297, QImage::Format_RGB32};
    img.fill(Qt::white);
    updateImage(img);

    setScene(scanScene.get());
}

ScanImageView::~ScanImageView() {}

void ScanImageView::updateImage(const QImage &_img) {
    imagePixmap = std::make_unique<QGraphicsPixmapItem>(QPixmap::fromImage(_img));
    imagePixmap->setZValue(0);

    overlay->updateImageRect(imagePixmap->boundingRect().toRect());

    scanScene->setSceneRect(imagePixmap->boundingRect().toRect());
    scanScene->addItem(imagePixmap.get());
}

void ScanImageView::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->fillRect(rect, Qt::darkGray);
}

double ScanImageView::getScale() const {
    // This assumes that we only ever translate and scale the scene
    // Once we rotate or scale x and y unevenly this logic falls apart.
    return transform().m11();
}

void ScanImageView::zoomFit() {
    const double oldScale = getScale();
    fitInView(imagePixmap.get(), Qt::KeepAspectRatio);
    const double newScale = getScale();
    if (oldScale != newScale) {
        emit zoomUpdated(newScale);
    }
}

void ScanImageView::zoomIn() { zoomSet(getScale() + 0.1); }
void ScanImageView::zoomOut() { zoomSet(getScale() - 0.1); }
void ScanImageView::zoomOriginal() { zoomSet(1.0); }

void ScanImageView::zoomSet(double _value) {
    _value = std::max(0.1, std::min(10.0, _value));
    if (getScale() == _value) {
        return;
    }

    const QTransform oldT = transform();
    QTransform       newT{};

    newT.scale(_value, _value);
    newT.translate(oldT.m31(), oldT.m32());

    setTransform(newT);
    emit zoomUpdated(_value);
}

void ScanImageView::selectionClear() {
    if (overlay->getSelection().isNull()) {
        return;
    }

    overlay->updateSelection(QRect());
    emit selectionChanged(overlay->getSelection());
}


QPoint ScanImageView::clipPoint(const QPoint &p) const {
    int x = std::min((int)imagePixmap->boundingRect().width(), std::max(0, p.x()));
    int y = std::min((int)imagePixmap->boundingRect().height(), std::max(0, p.y()));
    return {x, y};
}


void ScanImageView::mousePressEvent(QMouseEvent *event) {
    selectionStart = clipPoint(mapToScene(event->localPos().toPoint()).toPoint());
    overlay->updateSelection(QRect(selectionStart, selectionStart));
    QGraphicsView::mousePressEvent(event);
}

void ScanImageView::mouseMoveEvent(QMouseEvent *event) {
    QPoint p = clipPoint(mapToScene(event->localPos().toPoint()).toPoint());
    overlay->updateSelection(QRect(selectionStart, p).normalized());
    QGraphicsView::mouseMoveEvent(event);
}

void ScanImageView::mouseReleaseEvent(QMouseEvent *event) {
    emit selectionChanged(overlay->getSelection());
    QGraphicsView::mouseReleaseEvent(event);
}

void ScanImageView::updateSelection(QRect _newSelection) {
    overlay->updateSelection(_newSelection);
}

QRect ScanImageView::getSelection() const { return overlay->getSelection(); }

} // namespace qscan::gui
