#pragma once

#include <vector>
#include <cstdint>
#include <QImage>

namespace qscan::gui {

struct QImageContainer {
    std::vector<uint32_t> raw;
    QImage                img;

    QImageContainer() = default;
    QImageContainer(std::vector<uint32_t> &&_raw, QImage &&_img) : raw(_raw), img(_img) {}
    ~QImageContainer() = default;

    QImageContainer(const QImageContainer &) = delete;
    QImageContainer(QImageContainer &&)      = default;
    
    QImageContainer &operator=(const QImageContainer &) = delete;
    QImageContainer &operator=(QImageContainer &&)      = default;
};

} // namespace qscan::gui
