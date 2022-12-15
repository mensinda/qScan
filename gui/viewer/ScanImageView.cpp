#include "ScanImageView.hpp"
#include "qscan_log.hpp"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

using namespace qscan::lib;

namespace qscan::gui {

ScanImageView::ScanImageView(QWidget *parent) : QGraphicsView(parent) {
    image       = new QImage(QString::fromStdString("/home/daniel/tmp.png"));
    imagePixmap = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    imagePixmap->setZValue(0);

    overlay = new SelectionOverlay(image->rect());
    overlay->setZValue(1);
    overlay->updateSelection(QRect(image->width() / 4, image->height() / 4, image->width() / 2, image->height() / 2));

    scanScene = new QGraphicsScene(this);
    scanScene->setSceneRect(0, 0, image->width(), image->height());

    scanScene->addItem(imagePixmap);
    scanScene->addItem(overlay);

    setScene(scanScene);
}

ScanImageView::~ScanImageView() {}

void ScanImageView::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->fillRect(rect, QWidget::palette().color(QPalette::Shadow));
}

double ScanImageView::getScale() const {
    // This assumes that we only ever translate and scale the scene
    // Once we rotate or scale x and y unevenly this logic falls apart.
    return transform().m11();
}

void ScanImageView::zoomFit() {
    const double oldScale = getScale();
    fitInView(imagePixmap, Qt::KeepAspectRatio);
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
    emit selectionChanged();
}


QPoint ScanImageView::clipPoint(QPoint const& p) const {
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

} // namespace qscan::gui
