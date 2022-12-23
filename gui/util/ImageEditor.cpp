#include "ImageEditor.hpp"

#include <Magick++.h>
#include <ranges>

namespace {

Magick::Image qImage2Magick(const QImage &_image) {
    if (_image.format() != QImage::Format_RGBA8888) {
        throw std::runtime_error{"Image format is not Format_RGBA8888!"};
    }
    Magick::Blob blob{
        _image.constBits(),
        sizeof(uint32_t) * _image.width() * _image.height(),
    };
    Magick::Image image{
        blob,
        {(size_t)_image.width(), (size_t)_image.height()},
        8,
        "RGBA",
    };

    return image;
}

QImage magick2qImage(Magick::Image &_image) {
    Magick::Blob blob;
    _image.magick("RGBA");
    _image.write(&blob);
    QImage img{
        reinterpret_cast<const uchar *>(blob.data()),
        (int)_image.size().width(),
        (int)_image.size().height(),
        QImage::Format_RGBA8888,
    };
    return img.copy();
}

} // namespace

namespace qscan::gui {

QImage rotate(const QImage &_image, double _angle) {
    Magick::Image img = qImage2Magick(_image);
    img.rotate(_angle);
    return magick2qImage(img);
}

QImage crop(const QImage &_image, QRect _rect) {
    Magick::Image img = qImage2Magick(_image);
    img.chop(Magick::Geometry(_rect.x(), _rect.y()));
    img.crop(Magick::Geometry(_rect.width(), _rect.height()));
    return magick2qImage(img);
}


void saveImage(const QImage &_image, const std::string &_outPath) {
    Magick::Image img = qImage2Magick(_image);
    size_t        idx = _outPath.rfind('.');
    if (idx == std::string::npos) {
        img.magick("png");
    } else {
        img.magick(_outPath.substr(idx + 1));
    }
    img.write(_outPath);
}

void saveMultiImage(const std::vector<const QImage *> &_images, const std::string &_outPath) {
    std::vector<Magick::Image> imgs;
    imgs.reserve(_images.size());
    for (const QImage *i : _images) {
        imgs.push_back(qImage2Magick(*i));
    }
    Magick::writeImages(imgs.begin(), imgs.end(), _outPath);

    /*
    size_t idx = _outPath.rfind('.');
    if (idx == std::string::npos) {
        multiImage.magick("pdf");
    } else {
        multiImage.magick(_outPath.substr(idx + 1));
    }
    multiImage.write(_outPath);
     */
}


} // namespace qscan::gui
