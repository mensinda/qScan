#pragma once

#include <cstddef>
#include <cstdint>
#include <sane/sane.h>
#include <span>
#include <vector>

namespace qscan::lib {

class SaneImage {
  public:
    struct RGB8 {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct RGB16 {
        uint16_t r;
        uint16_t g;
        uint16_t b;
    };

    static_assert(sizeof(RGB8) == sizeof(uint8_t) * 3);
    static_assert(sizeof(RGB16) == sizeof(uint16_t) * 3);
    static_assert(alignof(RGB8) == sizeof(uint8_t));
    static_assert(alignof(RGB16) == sizeof(uint16_t));

  public:
    std::vector<uint8_t> rawDataC1;
    std::vector<uint8_t> rawDataC2;
    std::vector<uint8_t> rawDataC3;
    SANE_Parameters      parameters;

    bool isGray         = false;
    bool isMultiChannel = false;
    bool isSetup        = false;

  public:
    void addRawData(std::span<uint8_t> _raw, SANE_Frame _format);

    void setup(SANE_Parameters _parameters);

    size_t width();
    size_t height();
    size_t depth();

    std::vector<RGB8>  asRGB8();
    std::vector<RGB16> asRGB16();

  private:
    size_t commonRawSize();
};

} // namespace qscan::lib
