#include "SelectionOverlay.hpp"

#include <QPainter>

namespace qscan::gui {

SelectionOverlay::SelectionOverlay(QRect _imgBounds) : imgBounds(_imgBounds) {}

SelectionOverlay::~SelectionOverlay() {}

QRectF SelectionOverlay::boundingRect() const { return imgBounds; }

void SelectionOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    (void)painter;
    (void)option;
    (void)widget;

    if (selection.isNull()) {
        return;
    }

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setRenderHint(QPainter::Antialiasing, false);

    QRect rTop{0, 0, imgBounds.width(), selection.y()};
    QRect rBot{
        0,
        selection.y() + selection.height(),
        imgBounds.width(),
        imgBounds.height() - (selection.y() + selection.height()),
    };
    QRect rLeft{0, selection.y(), selection.x(), selection.height()};
    QRect rRight{
        selection.x() + selection.width(),
        selection.y(),
        imgBounds.width() - (selection.x() + selection.width()),
        selection.height(),
    };

    QColor bgColor{0, 0, 0, 192};
    painter->fillRect(rTop, bgColor);
    painter->fillRect(rBot, bgColor);
    painter->fillRect(rLeft, bgColor);
    painter->fillRect(rRight, bgColor);

    QLine lTop{selection.x(), selection.y(), selection.x() + selection.width(), selection.y()};
    QLine lBot{
        selection.x(),
        selection.y() + selection.height(),
        selection.x() + selection.width(),
        selection.y() + selection.height(),
    };
    QLine lLeft{selection.x(), selection.y(), selection.x(), selection.y() + selection.height()};
    QLine lRight{
        selection.x() + selection.width(),
        selection.y(),
        selection.x() + selection.width(),
        selection.y() + selection.height(),
    };

    QPen pen{Qt::yellow};
    pen.setWidth(0);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(lTop);
    painter->drawLine(lBot);
    painter->drawLine(lLeft);
    painter->drawLine(lRight);
}
void SelectionOverlay::updateSelection(QRect _selection) {
    selection = _selection;
    update();
}

void SelectionOverlay::updateImageRect(QRect _imgBounds) {
    imgBounds = _imgBounds;
    if (selection.isNull()) {
        return;
    }
    selection = selection.intersected(imgBounds);
}


} // namespace qscan::gui
