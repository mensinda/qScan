#pragma once

#include <QWidget>

namespace Ui {

class ImageViewer;

}

namespace qscan::gui {

class ScanImageView;

class ImageViewer : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::ImageViewer> ui;

  public:
    explicit ImageViewer(QWidget *_parent = nullptr);
    virtual ~ImageViewer();

    void updateImage(const QImage &_img);

    [[nodiscard]] ScanImageView *imageView();

  public slots:
    void selectionChanged(QRect _selectionRect);
    void updateSelection(QRect _newSelection);

  signals:
    void selectionUpdated(QRect _rect);
};

} // namespace qscan::gui
