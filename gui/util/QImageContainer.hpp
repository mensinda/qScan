#pragma once

#include "SaneImage.hpp"

#include <Magick++.h>
#include <cstdint>
#include <memory>
#include <vector>

#include <QImage>

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
