#pragma once

#include "SaneOption.hpp"

#include <optional>
#include <vector>

namespace qscan::lib {

class SaneOptionsWrapper {
  public:
    struct ScanArea {
        double topLeftX;
        double topLeftY;
        double bottomRightX;
        double bottomRightY;
    };

    struct ScanAreaData {
        ScanArea  current;
        ScanArea  min;
        ScanArea  max;
        SANE_Unit unit;
    };

    template <class T>
    struct GenericOptionData {
        T              current;
        std::vector<T> values;
        std::string    title;
        std::string    description;
        SANE_Unit      unit;
    };

  private:
    std::vector<SaneOption> *allOptions;

    // Option indexes of well known options
    int sourceIdx       = -1;
    int modeIdx         = -1;
    int resolutionIdx   = -1;
    int previewIdx      = -1;
    int bottomRightXIdx = -1;
    int bottomRightYIdx = -1;
    int topLeftXIdx     = -1;
    int topLeftYIdx     = -1;

  public:
    explicit SaneOptionsWrapper(std::vector<SaneOption> *options);
    virtual ~SaneOptionsWrapper();

    std::optional<GenericOptionData<std::string>> getSource();
    std::optional<GenericOptionData<std::string>> getMode();
    std::optional<GenericOptionData<double>>      getResolution();
    std::optional<ScanAreaData>                   getScanArea();
    std::optional<bool>                           getPreview();

    bool setSource(std::string source);
    bool setMode(std::string mode);
    bool setResolution(double resolution);
    bool setScanArea(ScanArea area);
    bool setPreview(bool preview);

    void refreshFilter();

    // No copy and move allowed

    SaneOptionsWrapper(const SaneOptionsWrapper &) = delete;
    SaneOptionsWrapper(SaneOptionsWrapper &&)      = delete;

    SaneOptionsWrapper &operator=(const SaneOptionsWrapper &) = delete;
    SaneOptionsWrapper &operator=(SaneOptionsWrapper &&)      = delete;
};

} // namespace qscan::lib
