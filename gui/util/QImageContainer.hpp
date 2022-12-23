#pragma once

#include <Magick++.h>
#include <QImage>
#include <cstdint>
#include <vector>

namespace qscan::gui {

struct QImageContainer {
    std::vector<uint32_t> raw;
    Magick::Blob          blob;
    QImage                img;

    QImageContainer() = default;
    QImageContainer(std::vector<uint32_t> &&_raw, const Magick::Blob& _blob, QImage &&_img)
        : raw(_raw), blob(_blob), img(_img) {}
    ~QImageContainer() = default;

    QImageContainer(const QImageContainer &) = delete;
    QImageContainer(QImageContainer &&)      = default;

    QImageContainer &operator=(const QImageContainer &) = delete;
    QImageContainer &operator=(QImageContainer &&)      = default;
};

} // namespace qscan::gui
