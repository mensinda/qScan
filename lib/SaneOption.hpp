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

    [[nodiscard]] const std::string   &getName() const { return name; }
    [[nodiscard]] const std::string   &getTitle() const { return title; }
    [[nodiscard]] const std::string   &getDesc() const { return desc; }
    [[nodiscard]] SANE_Constraint_Type getConstraintType() const { return constraintType; }
    [[nodiscard]] SANE_Value_Type      getType() const { return type; }
    [[nodiscard]] SANE_Unit            getUnit() const { return unit; }

    [[nodiscard]] const std::variant<RangeInt, RangeDouble, std::vector<SANE_Word>, std::vector<std::string>>
        &getConstraint() const {
        return constraint;
    }

    [[nodiscard]] const std::variant<bool, int, double, std::string> &getValue() const { return value; }

    [[nodiscard]] bool setValue(std::variant<bool, int, double, std::string> newValue);

    SaneOption(const SaneOption &) = delete;
    SaneOption(SaneOption &&)      = default;

    SaneOption &operator=(const SaneOption &) = delete;
    SaneOption &operator=(SaneOption &&)      = default;
};

} // namespace qscan::lib
