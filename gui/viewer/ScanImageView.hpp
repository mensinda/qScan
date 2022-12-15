#pragma once

#include "SelectionOverlay.hpp"
#include <QGraphicsView>

namespace qscan::gui {

class ScanImageView : public QGraphicsView {
    Q_OBJECT

  private:
    QGraphicsScene      *scanScene{};
    QImage              *image{};
    QGraphicsPixmapItem *imagePixmap{};
    SelectionOverlay    *overlay{};

    QPoint selectionStart{};

  public:
    explicit ScanImageView(QWidget *parent);
    virtual ~ScanImageView();

    double getScale() const;

  public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomOriginal();
    void zoomSet(double _value);

    void selectionClear();

  signals:
    void zoomUpdated(double _value);
    void selectionChanged();

  protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPoint clipPoint(const QPoint &p) const;
};

} // namespace qscan::gui
