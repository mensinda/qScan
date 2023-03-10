#include "SaneOption.hpp"

#include "SaneDevice.hpp"
#include "SaneException.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"

#include <algorithm>

namespace qscan::lib {

SaneOption::SaneOption(SaneDevice *_device, SANE_Int _index) : device(_device), index(_index) { reload(); }

void SaneOption::exceptional_control_option(SANE_Action action, void *value_raw, SANE_Int *info) {
    SANE_Status status = sane_control_option(device->getHandle(), index, action, value_raw, info);
    if (status != SANE_STATUS_GOOD) {
        std::string errorStatus =
            fmt::format("option {} (index {}) failed; type={}; uint={}; size={}; constraint_type={}; caps={}",
                        name,
                        index,
                        enum2str::toStr(type),
                        enum2str::toStr(unit),
                        size,
                        enum2str::toStr(constraintType),
                        enum2str::SaneOption_OptionCap_toStr(caps));
        switch (action) {
            case SANE_ACTION_GET_VALUE: throw SaneException(status, "Getting " + errorStatus);
            default: throw SaneException(status, "Setting " + errorStatus);
        }
    }
}

void SaneOption::reload() {
    const SANE_Option_Descriptor *optionDescriptor = sane_get_option_descriptor(device->getHandle(), index);

    name  = optionDescriptor->name ?: "";
    title = optionDescriptor->title ?: "";
    desc  = optionDescriptor->desc ?: "";

    type = optionDescriptor->type;
    unit = optionDescriptor->unit;

    caps           = optionDescriptor->cap;
    size           = optionDescriptor->size;
    constraintType = optionDescriptor->constraint_type;

    switch (constraintType) {
        case SANE_CONSTRAINT_RANGE:
            {
                const SANE_Range *range = optionDescriptor->constraint.range;
                if (type == SANE_TYPE_FIXED) {
                    constraint = RangeDouble{
                        SANE_UNFIX(range->min),
                        SANE_UNFIX(range->max),
                        SANE_UNFIX(range->quant),
                    };
                } else {
                    constraint = RangeInt{
                        range->min,
                        range->max,
                        range->quant,
                    };
                }
                break;
            }
        case SANE_CONSTRAINT_STRING_LIST:
            {
                std::vector<std::string> tmp;
                for (const SANE_String_Const *str = optionDescriptor->constraint.string_list; *str; str++) {
                    tmp.emplace_back(*str);
                }
                constraint = tmp;
                break;
            }
        case SANE_CONSTRAINT_WORD_LIST:
            {
                const SANE_Word *word_list  = optionDescriptor->constraint.word_list;
                const SANE_Word  clist_size = word_list[0];
                if (type == SANE_TYPE_INT) {
                    std::vector<int> tmp(clist_size);
                    for (int i = 1; i <= clist_size; ++i) {
                        tmp[i - 1] = word_list[i];
                    }
                    constraint = tmp;
                } else if (type == SANE_TYPE_FIXED) {
                    std::vector<double> tmp(clist_size);
                    for (int i = 1; i <= clist_size; ++i) {
                        tmp[i - 1] = SANE_UNFIX(word_list[i]);
                    }
                    constraint = tmp;
                } else {
                    throw std::runtime_error(fmt::format("Invalid type {} for constraint {}",
                                                         enum2str::toStr(type),
                                                         enum2str::toStr(constraintType)));
                }
                break;
            }
        case SANE_CONSTRAINT_NONE: break;
    }

    reloadValue();
}

consteval size_t constStrlen(const char *str) {
    const char *end;
    for (end = str; *end != 0; ++end)
        ;
    return end - str;
}

void SaneOption::reloadValue() {
    static_assert(sizeof(uint8_t) == 1);
    auto  raw_ptr = std::make_unique<uint8_t[]>(size);
    void *raw     = raw_ptr.get();

    // Do not fetch inactive values
    if ((caps & SANE_CAP_INACTIVE) == SANE_CAP_INACTIVE) {
        logger()->debug("[Sane]  -- {:<16}: {:<8} | {:<8} | {:<16} = {:<32} -- ({})",
                        name,
                        std::string_view{enum2str::toStr(type)}.substr(constStrlen("SANE_TYPE_")),
                        std::string_view{enum2str::toStr(unit)}.substr(constStrlen("SANE_UNIT_")),
                        std::string_view{enum2str::toStr(constraintType)}.substr(constStrlen("SANE_CONSTRAINT_")),
                        "<INACTIVE>",
                        enum2str::SaneOption_OptionCap_toStr(caps));
        switch (type) {
            case SANE_TYPE_INT: value = 0; break;
            case SANE_TYPE_FIXED: value = 0.0f; break;
            case SANE_TYPE_BOOL: value = false; break;
            case SANE_TYPE_STRING: value = ""; break;
            default: throw std::runtime_error("Unsupported option type " + enum2str::toStr(type));
        }
        return;
    }

    exceptional_control_option(SANE_ACTION_GET_VALUE, raw, nullptr);
    switch (type) {
        case SANE_TYPE_INT: value = *((SANE_Int *)raw); break;
        case SANE_TYPE_FIXED: value = SANE_UNFIX(*((SANE_Fixed *)raw)); break;
        case SANE_TYPE_BOOL: value = *((SANE_Bool *)raw) == SANE_TRUE; break;
        case SANE_TYPE_STRING: value = std::string{(SANE_String)raw}; break;
        default: throw std::runtime_error("Unsupported option type " + enum2str::toStr(type));
    }

    std::string valueStr = std::visit([](auto &&v) -> std::string { return fmt::format("{}", v); }, value);

    logger()->debug("[Sane]  -- {:<16}: {:<8} | {:<8} | {:<16} = {:<32} -- ({})",
                    name,
                    std::string_view{enum2str::toStr(type)}.substr(constStrlen("SANE_TYPE_")),
                    std::string_view{enum2str::toStr(unit)}.substr(constStrlen("SANE_UNIT_")),
                    std::string_view{enum2str::toStr(constraintType)}.substr(constStrlen("SANE_CONSTRAINT_")),
                    valueStr,
                    enum2str::SaneOption_OptionCap_toStr(caps));
}

bool SaneOption::setValue(value_t newValue) {
    if ((caps & SANE_CAP_INACTIVE) == SANE_CAP_INACTIVE) {
        logger()->warn("[Sane] Not setting INACTIVE option {}", name);
        return false;
    }

    SANE_Int info;
    switch (type) {
        case SANE_TYPE_INT:
            {
                SANE_Int val = get<int>(newValue);
                exceptional_control_option(SANE_ACTION_SET_VALUE, &val, &info);
                break;
            }
        case SANE_TYPE_FIXED:
            {
                SANE_Fixed val = SANE_FIX(get<double>(newValue));
                exceptional_control_option(SANE_ACTION_SET_VALUE, &val, &info);
                break;
            }
        case SANE_TYPE_BOOL:
            {
                SANE_Bool val = get<bool>(newValue) ? SANE_TRUE : SANE_FALSE;
                exceptional_control_option(SANE_ACTION_SET_VALUE, &val, &info);
                break;
            }
        case SANE_TYPE_STRING:
            {
                SANE_String val = const_cast<SANE_String>(get<std::string>(newValue).c_str());
                exceptional_control_option(SANE_ACTION_SET_VALUE, val, &info);
                break;
            }
        default: throw std::runtime_error("Unsupported option type " + enum2str::toStr(type));
    }


    if ((info & SANE_INFO_RELOAD_OPTIONS) == SANE_INFO_RELOAD_OPTIONS) {
        device->reload_options();
        return true;
    }

    reloadValue();
    return false;
}

} // namespace qscan::lib
