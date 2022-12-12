#include "SaneOptionsWrapper.hpp"
#include "constStringHash.hpp"
#include <algorithm>
#include <stdexcept>

namespace qscan::lib {

SaneOptionsWrapper::SaneOptionsWrapper(std::vector<SaneOption> *options) : allOptions(options) {}

SaneOptionsWrapper::~SaneOptionsWrapper() {}

bool isNumeric(const SaneOption &opt) { return opt.getType() == SANE_TYPE_INT || opt.getType() == SANE_TYPE_FIXED; }

void SaneOptionsWrapper::refreshFilter() {
    for (size_t i = 0; i < allOptions->size(); ++i) {
        SaneOption &opt = (*allOptions)[i];
        switch (fnv1aHash(opt.getName())) {
            case "tl-x"_h: topLeftXIdx = isNumeric(opt) ? (int)i : -1; break;
            case "tl-y"_h: topLeftYIdx = isNumeric(opt) ? (int)i : -1; break;
            case "br-x"_h: bottomRightXIdx = isNumeric(opt) ? (int)i : -1; break;
            case "br-y"_h: bottomRightYIdx = isNumeric(opt) ? (int)i : -1; break;
            case "mode"_h: modeIdx = opt.getType() == SANE_TYPE_STRING ? (int)i : -1; break;
            case "source"_h: sourceIdx = opt.getType() == SANE_TYPE_STRING ? (int)i : -1; break;
            case "resolution"_h: resolutionIdx = isNumeric(opt) ? (int)i : -1; break;
        }
    }
}

std::optional<SaneOptionsWrapper::ScanAreaData> SaneOptionsWrapper::getScanArea() {
    if (topLeftXIdx < 0 || topLeftYIdx < 0 || bottomRightXIdx < 0 || bottomRightYIdx < 0) { return {}; }

    // Just make my life a bit easier
    std::vector<SaneOption> &opts = *allOptions;

    if (opts[topLeftYIdx].getType() == SANE_TYPE_INT) {
        return ScanAreaData{
            ScanArea{
                     (double)get<int>(opts[bottomRightXIdx].getValue()),
                     (double)get<int>(opts[bottomRightYIdx].getValue()),
                     (double)get<int>(opts[topLeftXIdx].getValue()),
                     (double)get<int>(opts[topLeftYIdx].getValue()),
                     },
            ScanArea{
                     (double)get<SaneOption::RangeInt>(opts[bottomRightXIdx].getConstraint()).min,
                     (double)get<SaneOption::RangeInt>(opts[bottomRightYIdx].getConstraint()).min,
                     (double)get<SaneOption::RangeInt>(opts[topLeftXIdx].getConstraint()).min,
                     (double)get<SaneOption::RangeInt>(opts[topLeftYIdx].getConstraint()).min,
                     },
            ScanArea{
                     (double)get<SaneOption::RangeInt>(opts[bottomRightXIdx].getConstraint()).max,
                     (double)get<SaneOption::RangeInt>(opts[bottomRightYIdx].getConstraint()).max,
                     (double)get<SaneOption::RangeInt>(opts[topLeftXIdx].getConstraint()).max,
                     (double)get<SaneOption::RangeInt>(opts[topLeftYIdx].getConstraint()).max,
                     },
            opts[topLeftYIdx].getUnit(),
        };
    }

    return ScanAreaData{
        ScanArea{
                 get<double>(opts[bottomRightXIdx].getValue()),
                 get<double>(opts[bottomRightYIdx].getValue()),
                 get<double>(opts[topLeftXIdx].getValue()),
                 get<double>(opts[topLeftYIdx].getValue()),
                 },
        ScanArea{
                 get<SaneOption::RangeDouble>(opts[bottomRightXIdx].getConstraint()).min,
                 get<SaneOption::RangeDouble>(opts[bottomRightYIdx].getConstraint()).min,
                 get<SaneOption::RangeDouble>(opts[topLeftXIdx].getConstraint()).min,
                 get<SaneOption::RangeDouble>(opts[topLeftYIdx].getConstraint()).min,
                 },
        ScanArea{
                 get<SaneOption::RangeDouble>(opts[bottomRightXIdx].getConstraint()).max,
                 get<SaneOption::RangeDouble>(opts[bottomRightYIdx].getConstraint()).max,
                 get<SaneOption::RangeDouble>(opts[topLeftXIdx].getConstraint()).max,
                 get<SaneOption::RangeDouble>(opts[topLeftYIdx].getConstraint()).max,
                 },
        opts[topLeftYIdx].getUnit(),
    };
}

std::optional<SaneOptionsWrapper::GenericOptionData<std::string>> SaneOptionsWrapper::getSource() {
    if (sourceIdx < 0) { return {}; }

    SaneOption &opt = (*allOptions)[sourceIdx];
    if (opt.getConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
        return GenericOptionData<std::string>{get<std::string>(opt.getValue()),
                                              get<std::vector<std::string>>(opt.getConstraint())};
    }

    return GenericOptionData<std::string>{get<std::string>(opt.getValue()), {}};
}

std::optional<SaneOptionsWrapper::GenericOptionData<std::string>> SaneOptionsWrapper::getMode() {
    if (modeIdx < 0) { return {}; }

    SaneOption &opt = (*allOptions)[modeIdx];
    if (opt.getConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
        return GenericOptionData<std::string>{get<std::string>(opt.getValue()),
                                              get<std::vector<std::string>>(opt.getConstraint())};
    }

    return GenericOptionData<std::string>{get<std::string>(opt.getValue()), {}};
}

std::optional<SaneOptionsWrapper::GenericOptionData<double>> SaneOptionsWrapper::getResolution() {
    if (resolutionIdx < 0) { return {}; }

    SaneOption &opt = (*allOptions)[modeIdx];
    if (opt.getConstraintType() == SANE_CONSTRAINT_WORD_LIST) {
        std::vector<SANE_Word> tmp = get<std::vector<SANE_Word>>(opt.getConstraint());
        std::vector<double>    res;

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(res), [&opt](SANE_Word w) {
            if (opt.getType() == SANE_TYPE_FIXED) {
                return SANE_UNFIX(w);
            } else {
                return (double)w;
            }
        });

        return GenericOptionData<double>{get<double>(opt.getValue()), res};
    }

    return GenericOptionData<double>{get<double>(opt.getValue()), {}};
}

bool SaneOptionsWrapper::setSource(std::string source) {
    if (sourceIdx < 0) { throw std::runtime_error("Unable to set source (option does not exist)"); }

    if ((*allOptions)[sourceIdx].setValue(source)) {
        refreshFilter();
        return true;
    }

    return false;
}

bool SaneOptionsWrapper::setMode(std::string mode) {
    if (modeIdx < 0) { throw std::runtime_error("Unable to set mode (option does not exist)"); }

    if ((*allOptions)[modeIdx].setValue(mode)) {
        refreshFilter();
        return true;
    }

    return false;
}

bool SaneOptionsWrapper::setResolution(double resolution) {
    if (resolutionIdx < 0) { throw std::runtime_error("Unable to set resolution (option does not exist)"); }

    if ((*allOptions)[resolutionIdx].setValue(resolution)) {
        refreshFilter();
        return true;
    }

    return false;
}

bool SaneOptionsWrapper::setScanArea(SaneOptionsWrapper::ScanArea area) {
    if (bottomRightXIdx < 0 || bottomRightYIdx < 0 || topLeftXIdx < 0 || topLeftYIdx < 0) {
        throw std::runtime_error("Unable to set area (option does not exist)");
    }

    bool refresh = false;
    refresh |= (*allOptions)[bottomRightXIdx].setValue(area.bottomRightX);
    refresh |= (*allOptions)[bottomRightYIdx].setValue(area.bottomRightY);
    refresh |= (*allOptions)[topLeftXIdx].setValue(area.topLeftX);
    refresh |= (*allOptions)[topLeftYIdx].setValue(area.topLeftY);

    if (refresh) { refreshFilter(); }

    return refresh;
}

} // namespace qscan::lib
