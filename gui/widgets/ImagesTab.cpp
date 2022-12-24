#include "ImagesTab.hpp"
#include "util/ImageEditor.hpp"
#include "ScanImageView.hpp"
#include "qscan_log.hpp"
#include "ui_ImagesTab.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace qscan::lib;

namespace qscan::gui {

ImagesTab::ImagesTab(QWidget *_parent) : QWidget(_parent), ui(new Ui::ImagesTab) {
    ui->setupUi(this);

    ui->imageViewer->setEnabled(false);
    ui->controlBar->setEnabled(false);

    // Undo and Redo is not implemented yet --> hide the buttons
    ui->seperatorUndo->hide();
    ui->btnUndo->hide();
    ui->btnRedo->hide();
}

ImagesTab::~ImagesTab() {}

void ImagesTab::skipUpdated(int _value) { ui->selectOffset->setMaximum(_value); }
void ImagesTab::selectionHasUpdated(QRect _newSelection) { ui->btnCrop->setEnabled(!_newSelection.isNull()); }

void ImagesTab::doSelectAll() {
    doSelectNone();
    for (int i = ui->selectOffset->value(); i < ui->listWidget->count(); i += ui->selectSkip->value() + 1) {
        ui->listWidget->item(i)->setSelected(true);
    }
}

void ImagesTab::doSelectNone() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        ui->listWidget->item(i)->setSelected(false);
    }
}

void ImagesTab::doCrop() {
    QRect selection = ui->imageViewer->imageView()->getSelection();
    if (selection.isNull()) {
        return;
    }
    for (auto *img : currentlyActiveImages()) {
        img->img.img  = crop(img->img.img, selection);
        img->modified = true;
    }
    ui->imageViewer->updateImage(currentImage()->img.img);
}

void ImagesTab::doRotateP90() {
    for (auto *img : currentlyActiveImages()) {
        img->img.img  = rotate(img->img.img, 90.0);
        img->modified = true;
    }
    ui->imageViewer->updateImage(currentImage()->img.img);
}

void ImagesTab::doRotateM90() {
    for (auto *img : currentlyActiveImages()) {
        img->img.img  = rotate(img->img.img, -90.0);
        img->modified = true;
    }
    ui->imageViewer->updateImage(currentImage()->img.img);
}

void ImagesTab::doRotate180() {
    for (auto *img : currentlyActiveImages()) {
        img->img.img  = rotate(img->img.img, 180.0);
        img->modified = true;
    }
    ui->imageViewer->updateImage(currentImage()->img.img);
}

void ImagesTab::doSave() {
    ImageData *image = currentImage();
    if (!image) {
        return;
    }

    if (image->lastSaveLocation.isEmpty()) {
        image->lastSaveLocation = image->item->text();
    }

    QString saveFileLocation = QFileDialog::getSaveFileName(
        this,
        tr("Save the currently displayed image in the image viewer"),
        image->lastSaveLocation,
        tr("PNG(*.png);;JPEG(*.jpg);;GIF(*.gif);;BMP(*.bmp);;TIFF(*.tiff);;PSD(*.psd);;PPM(*.ppm);;PBM(*.pbm)"));

    if (saveFileLocation.isNull()) {
        return;
    }

    image->lastSaveLocation = saveFileLocation;
    saveImage(image->img.img, saveFileLocation.toStdString());
    image->modified = false;
}

void ImagesTab::doSaveAll() {
    std::vector<const QImage *> imgPtrs;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        int              idx  = item->data(Qt::UserRole).toInt();
        imgPtrs.push_back(&images[idx].img.img);
        images[idx].modified = false;
    }

    QString saveFileLocation =
        QFileDialog::getSaveFileName(this,
                                     tr("Create a multi-image document from all images in the tab"),
                                     QString(),
                                     tr("PDF(*.pdf);;DVI(*.dvi)"));

    if (saveFileLocation.isNull()) {
        return;
    }

    saveMultiImage(imgPtrs, saveFileLocation.toStdString());
}

void ImagesTab::currentImageUpdated(QListWidgetItem *_cur, QListWidgetItem *_prev) {
    (void)_prev;

    if (_cur == nullptr) {
        return;
    }

    ui->imageViewer->setEnabled(true);
    ui->imageViewer->updateImage(currentImage()->img.img);
    ui->imageViewer->imageView()->zoomFit();
}


void ImagesTab::doUndo() {
    // TODO implement!
    logger()->warn("Undo is not implemented yet!");
}

void ImagesTab::doRedo() {
    // TODO implement!
    logger()->warn("Redo is not implemented yet!");
}

void ImagesTab::addImage(QImageContainer &&_container) {
    images.push_back({
        std::move(_container),
        true,                                              // modified
        QString(),                                         // lastSaveLocation
        std::make_unique<QListWidgetItem>(ui->listWidget), // item
    });
    QListWidgetItem *item = images.back().item.get();
    item->setText(tr("Image %1").arg(QString::number(images.size())));
    item->setData(Qt::UserRole, QVariant((int)images.size() - 1));
    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemNeverHasChildren);
    ui->listWidget->addItem(item);
    ui->listWidget->clearSelection();
    ui->listWidget->setCurrentItem(item);
    ui->controlBar->setEnabled(true);
}

ImagesTab::ImageData *ImagesTab::currentImage() {
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item) {
        return nullptr;
    }

    int imgIdx = item->data(Qt::UserRole).toInt();
    return &images[imgIdx];
}

void ImagesTab::doDelete() {
    std::vector<ImageData *> selectedItems = currentlyActiveImages();
    if (selectedItems.empty()) {
        return;
    }
    if (selectedItems.size() > 1) {
        QMessageBox::StandardButton res = QMessageBox::warning(
            this,
            tr("More than one image selected"),
            tr("More than one item is selected. Are you sure that you want to delete all %1 items?")
                .arg(QString::number(selectedItems.size())),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (res != QMessageBox::Yes) {
            return;
        }
    }

    if (areAnySelectedImagesModified()) {
        QMessageBox::StandardButton res =
            QMessageBox::warning(this,
                                 tr("Unsaved images"),
                                 tr("Not all images have been saved. Do you still want to remove them?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);

        if (res != QMessageBox::Yes) {
            return;
        }
    }

    for (ImageData *imageData : selectedItems) {
        // Do not remove to keep the indexes stable. The additional
        // memory overhead should be minimal since the image data
        // itself is cleared.
        imageData->item     = nullptr;
        imageData->modified = false;
        imageData->img.img  = QImage{};
        imageData->img.raw  = nullptr;
    }

    if (ui->listWidget->count() == 0) {
        ui->controlBar->setEnabled(false);
        ui->imageViewer->setEnabled(false);
    }
}

bool ImagesTab::areAnyImagesModified() {
    return std::ranges::any_of(images, [](const ImageData &item) { return item.modified; });
}
bool ImagesTab::areAnySelectedImagesModified() {
    return std::ranges::any_of(currentlyActiveImages(), [](const ImageData *item) { return item->modified; });
}

std::vector<ImagesTab::ImageData *> ImagesTab::currentlyActiveImages() {
    std::vector<ImageData *> res{};
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.empty()) {
        QListWidgetItem *current = ui->listWidget->currentItem();
        if (!current) {
            return {};
        }
        selectedItems.push_back(current);
    }

    for (const QListWidgetItem *item : selectedItems) {
        res.push_back(&images[item->data(Qt::UserRole).toInt()]);
    }

    return res;
}

} // namespace qscan::gui
