#include "SaneImage.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"
#include <cstring>
#include <stdexcept>

namespace qscan::lib {

void SaneImage::setup(SANE_Parameters _parameters) {
    if (isSetup) {
        return;
    }
    parameters = _parameters;
    switch (_parameters.format) {
        case SANE_FRAME_GRAY:
            isMultiChannel = false;
            isGray         = true;
            rawDataC1.reserve(parameters.lines * parameters.bytes_per_line);
            break;
        case SANE_FRAME_RGB:
            isMultiChannel = false;
            isGray         = false;
            rawDataC1.reserve(parameters.lines * parameters.bytes_per_line);
            break;
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
            isMultiChannel = true;
            isGray         = false;
            rawDataC1.reserve(parameters.lines * parameters.bytes_per_line);
            rawDataC2.reserve(parameters.lines * parameters.bytes_per_line);
            rawDataC3.reserve(parameters.lines * parameters.bytes_per_line);
            break;
        default: throw std::runtime_error("Unknown format: " + enum2str::toStr(parameters.format));
    }
    isSetup = true;
}

void SaneImage::addRawData(std::span<uint8_t> _raw, SANE_Frame _format) {
    size_t offset;
    switch (_format) {
        case SANE_FRAME_GRAY:
        case SANE_FRAME_RGB:
        case SANE_FRAME_RED:
            offset = rawDataC1.size();
            rawDataC1.resize(offset + _raw.size());
            memcpy(rawDataC1.data() + offset, _raw.data(), _raw.size());
            break;
        case SANE_FRAME_GREEN:
            offset = rawDataC2.size();
            rawDataC2.resize(offset + _raw.size());
            memcpy(rawDataC2.data() + offset, _raw.data(), _raw.size());
            break;
        case SANE_FRAME_BLUE:
            offset = rawDataC3.size();
            rawDataC3.resize(offset + _raw.size());
            memcpy(rawDataC3.data() + offset, _raw.data(), _raw.size());
            break;
        default: throw std::runtime_error("Unknown format: " + enum2str::toStr(_format));
    }
}

size_t SaneImage::width() const { return parameters.pixels_per_line; }
size_t SaneImage::height() const { return commonRawSize() / parameters.bytes_per_line; }
size_t SaneImage::depth() const { return parameters.depth; }

std::unique_ptr<SaneImage::RGB8[]> SaneImage::asRGB8() {
    size_t s   = size();
    auto   res = std::make_unique<RGB8[]>(s);
    if (depth() > 8) {
        const auto data = asRGB16();
        for (size_t i = 0; i < s; ++i) {
            res[i].r = data[i].r >> 8;
            res[i].g = data[i].g >> 8;
            res[i].b = data[i].b >> 8;
        }
        return res;
    }

    if (depth() == 8 && isGray) {
        if (s > rawDataC1.size()) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s, rawDataC1.size()));
        }
        for (size_t i = 0; i < s; ++i) {
            res[i].r = rawDataC1[i];
            res[i].g = rawDataC1[i];
            res[i].b = rawDataC1[i];
        }
    } else if (depth() == 1 && isGray) {
        if (s > rawDataC1.size() * 8) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s, rawDataC1.size() * 8));
        }
        for (size_t i = 0; i < rawDataC1.size(); ++i) {
            uint8_t raw      = rawDataC1[i];
            res[i * 8 + 0].r = raw & 0b10000000 ? 0xFF : 0x00;
            res[i * 8 + 0].g = raw & 0b10000000 ? 0xFF : 0x00;
            res[i * 8 + 0].b = raw & 0b10000000 ? 0xFF : 0x00;

            res[i * 8 + 1].r = raw & 0b01000000 ? 0xFF : 0x00;
            res[i * 8 + 1].g = raw & 0b01000000 ? 0xFF : 0x00;
            res[i * 8 + 1].b = raw & 0b01000000 ? 0xFF : 0x00;

            res[i * 8 + 2].r = raw & 0b00100000 ? 0xFF : 0x00;
            res[i * 8 + 2].g = raw & 0b00100000 ? 0xFF : 0x00;
            res[i * 8 + 2].b = raw & 0b00100000 ? 0xFF : 0x00;

            res[i * 8 + 3].r = raw & 0b00010000 ? 0xFF : 0x00;
            res[i * 8 + 3].g = raw & 0b00010000 ? 0xFF : 0x00;
            res[i * 8 + 3].b = raw & 0b00010000 ? 0xFF : 0x00;

            res[i * 8 + 4].r = raw & 0b00001000 ? 0xFF : 0x00;
            res[i * 8 + 4].g = raw & 0b00001000 ? 0xFF : 0x00;
            res[i * 8 + 4].b = raw & 0b00001000 ? 0xFF : 0x00;

            res[i * 8 + 5].r = raw & 0b00000100 ? 0xFF : 0x00;
            res[i * 8 + 5].g = raw & 0b00000100 ? 0xFF : 0x00;
            res[i * 8 + 5].b = raw & 0b00000100 ? 0xFF : 0x00;

            res[i * 8 + 6].r = raw & 0b00000010 ? 0xFF : 0x00;
            res[i * 8 + 6].g = raw & 0b00000010 ? 0xFF : 0x00;
            res[i * 8 + 6].b = raw & 0b00000010 ? 0xFF : 0x00;

            res[i * 8 + 7].r = raw & 0b00000001 ? 0xFF : 0x00;
            res[i * 8 + 7].g = raw & 0b00000001 ? 0xFF : 0x00;
            res[i * 8 + 7].b = raw & 0b00000001 ? 0xFF : 0x00;
        }
    } else if (depth() == 8 && !isMultiChannel) {
        if (s * 3 > rawDataC1.size()) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s * 3, rawDataC1.size()));
        }
        memcpy(res.get(), rawDataC1.data(), s * 3);
    } else if (depth() == 8 && isMultiChannel) {
        if (s > commonRawSize()) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s, commonRawSize()));
        }
        for (size_t i = 0; i < s; ++i) {
            res[i].r = rawDataC1[i];
            res[i].g = rawDataC2[i];
            res[i].b = rawDataC3[i];
        }
    } else {
        throw std::runtime_error("Unsupported depth: " + std::to_string(depth()));
    }

    return res;
}

std::unique_ptr<SaneImage::RGB16[]> SaneImage::asRGB16() {
    size_t s   = size();
    auto   res = std::make_unique<RGB16[]>(s);
    if (depth() < 16) {
        const auto data = asRGB8();
        for (size_t i = 0; i < s; ++i) {
            res[i].r = data[i].r << 8;
            res[i].g = data[i].g << 8;
            res[i].b = data[i].b << 8;
        }
        return res;
    }

    if (depth() == 16 && !isMultiChannel) {
        if (s * 6 > rawDataC1.size()) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s * 6, rawDataC1.size()));
        }
        memcpy(res.get(), rawDataC1.data(), s * 6);
    } else if (depth() == 16 && isMultiChannel) {
        if (s * 2 > commonRawSize()) {
            throw std::runtime_error(fmt::format("Not enough data: Expected {} but got {}", s * 2, commonRawSize()));
        }
        std::span<uint16_t> dataC1{reinterpret_cast<uint16_t *>(rawDataC1.data()), rawDataC1.size() / 2};
        std::span<uint16_t> dataC2{reinterpret_cast<uint16_t *>(rawDataC2.data()), rawDataC2.size() / 2};
        std::span<uint16_t> dataC3{reinterpret_cast<uint16_t *>(rawDataC3.data()), rawDataC3.size() / 2};
        for (size_t i = 0; i < s; ++i) {
            res[i].r = dataC1[i];
            res[i].g = dataC2[i];
            res[i].b = dataC3[i];
        }
    } else {
        throw std::runtime_error("Unsupported depth: " + std::to_string(depth()));
    }

    return res;
}

size_t SaneImage::commonRawSize() const {
    if (!isMultiChannel) {
        return rawDataC1.size();
    }
    return std::min(rawDataC1.size(), std::min(rawDataC2.size(), rawDataC3.size()));
}

size_t SaneImage::size() const { return width() * height(); }

std::unique_ptr<SaneImage::RGBA8[]> SaneImage::asRGBA8() {
    const auto data = asRGB8();
    auto       res  = std::make_unique<RGBA8[]>(size());

    for (size_t i = 0; i < size(); ++i) {
        res[i] = {data[i].r, data[i].g, data[i].b, std::numeric_limits<uint8_t>::max()};
    }

    return res;
}

std::unique_ptr<SaneImage::RGBA16[]> SaneImage::asRGBA16() {
    const auto data = asRGB16();
    auto       res  = std::make_unique<RGBA16[]>(size());

    for (size_t i = 0; i < size(); ++i) {
        res[i] = {data[i].r, data[i].g, data[i].b, std::numeric_limits<uint16_t>::max()};
    }

    return res;
}

} // namespace qscan::lib
