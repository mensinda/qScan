#pragma once

#include "SaneImage.hpp"
#include <Magick++.h>
#include <QImage>
#include <cstdint>
#include <memory>
#include <vector>

namespace qscan::gui {

struct QImageContainer {
    using RGBA8 = lib::SaneImage::RGBA8;

    std::unique_ptr<RGBA8[]> raw;
    Magick::Blob             blob;
    QImage                   img;

    QImageContainer() = default;
    QImageContainer(std::unique_ptr<RGBA8[]> &&_raw, const Magick::Blob &_blob, QImage &&_img)
        : raw(std::move(_raw)), blob(_blob), img(_img) {}
    ~QImageContainer() = default;

    QImageContainer(const QImageContainer &) = delete;
    QImageContainer(QImageContainer &&)      = default;

    QImageContainer &operator=(const QImageContainer &) = delete;
    QImageContainer &operator=(QImageContainer &&)      = default;
};

} // namespace qscan::gui
