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
            isGray = true;
            rawDataC1.reserve(parameters.lines * parameters.bytes_per_line);
            break;
        case SANE_FRAME_RGB:
            isMultiChannel = false;
            isGray = false;
            rawDataC1.reserve(parameters.lines * parameters.bytes_per_line);
            break;
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
            isMultiChannel = true;
            isGray = false;
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

size_t SaneImage::width() { return parameters.pixels_per_line; }
size_t SaneImage::height() { return commonRawSize() / parameters.bytes_per_line; }
size_t SaneImage::depth() { return parameters.depth; }

std::vector<SaneImage::RGB8> SaneImage::asRGB8() {
    std::vector<RGB8> res;
    if (depth() > 8) {
        const std::vector<RGB16> data = asRGB16();
        res.resize(data.size());

        for (size_t i = 0; i < res.size(); ++i) {
            res[i].r = data[i].r >> 8;
            res[i].g = data[i].g >> 8;
            res[i].b = data[i].b >> 8;
        }

        return res;
    }

    if (depth() == 8 && isGray) {
        res.resize(rawDataC1.size());
        for (size_t i = 0; i < res.size(); ++i) {
            res[i].r = rawDataC1[i];
            res[i].g = rawDataC1[i];
            res[i].b = rawDataC1[i];
        }
    } else if (depth() == 1 && isGray) {
        res.resize(rawDataC1.size() * 8);
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
        res.resize(rawDataC1.size() / 3);
        memcpy(res.data(), rawDataC1.data(), rawDataC1.size());
    } else if (depth() == 8 && isMultiChannel) {
        res.resize(commonRawSize());
        for (size_t i = 0; i < res.size(); ++i) {
            res[i].r = rawDataC1[i];
            res[i].g = rawDataC2[i];
            res[i].b = rawDataC3[i];
        }
    } else {
        throw std::runtime_error("Unsupported depth: " + std::to_string(depth()));
    }

    return res;
}

std::vector<SaneImage::RGB16> SaneImage::asRGB16() {
    std::vector<RGB16> res;
    if (depth() < 16) {
        const std::vector<RGB8> data = asRGB8();
        res.resize(data.size());

        for (size_t i = 0; i < res.size(); ++i) {
            const RGB8 &orig = data[i];
            RGB16      &dest = res[i];

            dest.r = orig.r << 8;
            dest.g = orig.g << 8;
            dest.b = orig.b << 8;
        }

        return res;
    }

    if (depth() == 16 && !isMultiChannel) {
        res.resize(rawDataC1.size() / 6);
        std::span<uint16_t> dataC1{reinterpret_cast<uint16_t *>(rawDataC1.data()), rawDataC1.size() / 2};
        for (size_t i = 0; i < res.size(); ++i) {
            res[i].r = dataC1[i * 3 + 0];
            res[i].g = dataC1[i * 3 + 1];
            res[i].b = dataC1[i * 3 + 2];
        }
    } else if (depth() == 16 && isMultiChannel) {
        res.resize(commonRawSize() / 2);
        std::span<uint16_t> dataC1{reinterpret_cast<uint16_t *>(rawDataC1.data()), rawDataC1.size() / 2};
        std::span<uint16_t> dataC2{reinterpret_cast<uint16_t *>(rawDataC2.data()), rawDataC2.size() / 2};
        std::span<uint16_t> dataC3{reinterpret_cast<uint16_t *>(rawDataC3.data()), rawDataC3.size() / 2};
        for (size_t i = 0; i < res.size(); ++i) {
            res[i].r = dataC1[i];
            res[i].g = dataC2[i];
            res[i].b = dataC3[i];
        }
    } else {
        throw std::runtime_error("Unsupported depth: " + std::to_string(depth()));
    }

    return res;
}

size_t SaneImage::commonRawSize() {
    if (!isMultiChannel) {
        return rawDataC1.size();
    }
    return std::min(rawDataC1.size(), std::min(rawDataC2.size(), rawDataC3.size()));
}

} // namespace qscan::lib
