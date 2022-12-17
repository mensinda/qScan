#pragma once

#include "SelectionOverlay.hpp"
#include <QGraphicsView>

namespace qscan::gui {

class ScanImageView : public QGraphicsView {
    Q_OBJECT

  private:
    std::unique_ptr<QGraphicsScene>      scanScene;
    std::unique_ptr<QGraphicsPixmapItem> imagePixmap;
    std::unique_ptr<SelectionOverlay>    overlay;

    QPoint selectionStart{};

  public:
    explicit ScanImageView(QWidget *parent);
    virtual ~ScanImageView();

    [[nodiscard]] double getScale() const;

    void updateImage(const QImage &_img);
    void updateSelection(QRect _newSelection);

  public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomOriginal();
    void zoomSet(double _value);

    void selectionClear();

  signals:
    void zoomUpdated(double _value);
    void selectionChanged(QRect _selectionRect);

  protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    [[nodiscard]] QPoint clipPoint(const QPoint &p) const;
};

} // namespace qscan::gui
