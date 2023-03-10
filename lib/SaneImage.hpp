#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
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

    struct RGBA8 {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    struct RGB16 {
        uint16_t r;
        uint16_t g;
        uint16_t b;
    };

    struct RGBA16 {
        uint16_t r;
        uint16_t g;
        uint16_t b;
        uint16_t a;
    };

    static_assert(sizeof(RGB8) == sizeof(uint8_t) * 3);
    static_assert(sizeof(RGB16) == sizeof(uint16_t) * 3);
    static_assert(sizeof(RGBA8) == sizeof(uint8_t) * 4);
    static_assert(sizeof(RGBA16) == sizeof(uint16_t) * 4);
    static_assert(alignof(RGB8) == sizeof(uint8_t));
    static_assert(alignof(RGB16) == sizeof(uint16_t));
    static_assert(alignof(RGBA8) == sizeof(uint8_t));
    static_assert(alignof(RGBA16) == sizeof(uint16_t));

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

    [[nodiscard]] size_t width() const;
    [[nodiscard]] size_t height() const;
    [[nodiscard]] size_t depth() const;
    [[nodiscard]] size_t size() const;

    [[nodiscard]] std::unique_ptr<RGB8[]>   asRGB8();
    [[nodiscard]] std::unique_ptr<RGB16[]>  asRGB16();
    [[nodiscard]] std::unique_ptr<RGBA8[]>  asRGBA8();
    [[nodiscard]] std::unique_ptr<RGBA16[]> asRGBA16();

  private:
    size_t commonRawSize() const;
};

} // namespace qscan::lib
