#pragma once

#include "util/QImageContainer.hpp"
#include <QListWidgetItem>
#include <QWidget>
#include <memory>

namespace Ui {
class ImagesTab;
}

namespace qscan::gui {

class ImagesTab : public QWidget {
    Q_OBJECT

  public:
    struct ImageData {
        QImageContainer img;
        bool            modified;
        QString         lastSaveLocation;

        std::unique_ptr<QListWidgetItem> item;
    };

  private:
    std::unique_ptr<Ui::ImagesTab> ui;
    std::vector<ImageData>         images;

  public:
    explicit ImagesTab(QWidget *_parent);
    virtual ~ImagesTab();

    void addImage(QImageContainer &&_container);

    [[nodiscard]] ImageData *currentImage();

    [[nodiscard]] std::vector<ImageData *> currentlyActiveImages();

    [[nodiscard]] bool areAnyImagesModified();
    [[nodiscard]] bool areAnySelectedImagesModified();

  public slots:
    void doSelectAll();
    void skipUpdated(int _value);
    void doUndo();
    void doRedo();
    void doCrop();
    void doRotateP90();
    void doRotateM90();
    void doRotate180();
    void doSave();
    void doSaveAll();
    void doDelete();
    void currentImageUpdated(QListWidgetItem *_cur, QListWidgetItem *_prev);
    void selectionHasUpdated(QRect _newSelection);
};

} // namespace qscan::gui
