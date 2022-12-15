#pragma once

#include <QWidget>

namespace Ui {

class ImageViewer;

}

namespace qscan::gui {

class ImageViewer : public QWidget {
    Q_OBJECT

  private:
    std::unique_ptr<Ui::ImageViewer> ui;

  public:
    explicit ImageViewer(QWidget *_parent = nullptr);
    virtual ~ImageViewer();

  public slots:
    void selectionChanged(QRect _selectionRect);

};

}
