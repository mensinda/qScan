#pragma once

#include <QImage>

namespace qscan::gui {

[[nodiscard]] QImage rotate(const QImage &_image, double _angle);
[[nodiscard]] QImage crop(const QImage &_image, QRect _rect);

void saveImage(const QImage &_image, const std::string &_outPath);
void saveMultiImage(const std::vector<const QImage *> &_images, const std::string &_outPath);

} // namespace qscan::gui
