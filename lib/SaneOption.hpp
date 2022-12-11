#pragma once

#include <sane/sane.h>
#include <string>
#include <variant>
#include <vector>

namespace qscan::lib {

class SaneDevice;

class SaneOption {
  public:
    enum OptionCap {
        SOFT_SELECT = SANE_CAP_SOFT_SELECT,
        HARD_SELECT = SANE_CAP_HARD_SELECT,
        SOFT_DETECT = SANE_CAP_SOFT_DETECT,
        EMULATED    = SANE_CAP_EMULATED,
        AUTOMATIC   = SANE_CAP_AUTOMATIC,
        INACTIVE    = SANE_CAP_INACTIVE,
        ADVANCED    = SANE_CAP_ADVANCED,
    };

    struct RangeInt {
        SANE_Word min;
        SANE_Word max;
        SANE_Word step;
    };

    struct RangeDouble {
        double min;
        double max;
        double step;
    };

  private:
    SaneDevice *device;
    SANE_Int    index;
    std::string name;
    std::string title;
    std::string desc;

    SANE_Constraint_Type constraintType;
    SANE_Value_Type      type;
    SANE_Unit            unit;

    SANE_Int caps;
    SANE_Int size;

    std::variant<RangeInt, RangeDouble, std::vector<SANE_Word>, std::vector<std::string>> constraint;
    std::variant<bool, int, double, std::string>                                          value;

  private:
    void exceptional_control_option(SANE_Action action, void *value, SANE_Int *info);

  public:
    explicit SaneOption(SaneDevice *_device, SANE_Int _index, const SANE_Option_Descriptor *optionDescriptor);
    virtual ~SaneOption() = default;

    void reloadValue();

    SaneOption(const SaneOption &) = delete;
    SaneOption(SaneOption &&)      = default;

    SaneOption &operator=(const SaneOption &) = delete;
    SaneOption &operator=(SaneOption &&)      = default;
};

} // namespace qscan::lib
