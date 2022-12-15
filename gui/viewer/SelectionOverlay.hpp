#pragma once

#include <QGraphicsItem>

namespace qscan::gui {

class SelectionOverlay : public QGraphicsItem {
  private:
    QRect imgBounds;
    QRect selection{};

  public:
    explicit SelectionOverlay(QRect _imgBounds);
    virtual ~SelectionOverlay();

    /**
     * Change the currently selected area.
     *
     * Calling this function with a null rect will disable the overlay.
     *
     * @param _selection The new selection rect.
     */
    void updateSelection(QRect _selection);

    void updateImageRect(QRect _imgBounds);

    [[nodiscard]] const QRect &getSelection() const { return selection; }
    [[nodiscard]] QRectF       boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

} // namespace qscan::gui
