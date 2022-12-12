#include "SaneDevice.hpp"
#include "SaneException.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"

namespace qscan::lib {

SaneDevice::SaneDevice(const SANE_Device *device)
    : name(device->name), vendor(device->vendor), model(device->model), type(device->type), options(&rawOptions) {}

SaneDevice::~SaneDevice() {
    if (handle) {
        logger()->info("[Sane] Closing device {}", name);
        sane_close(handle);
    }
}

void SaneDevice::connect() {
    logger_t log = logger();

    log->info("[Sane] Connecting to {}", name);

    SANE_Status status = sane_open(name.c_str(), &handle);
    if (status != SANE_STATUS_GOOD) {
        handle = nullptr;
        throw SaneException(status, "Failed to connect to " + name);
    }

    reload_options();

    log->info("[Sane] Connected to {}", name);
}

void SaneDevice::reload_options() {
    logger_t log = logger();

    rawOptions.clear();

    SANE_Int     nopts  = 0;
    SANE_Status status = sane_control_option(handle, 0, SANE_ACTION_GET_VALUE, &nopts, nullptr);

    if (status != SANE_STATUS_GOOD) { throw SaneException(status, "Failed to get the number of options"); }

    log->info("[Sane] Loading {} options:", nopts);

    for (SANE_Int i = 1; i < nopts; ++i) {
        const SANE_Option_Descriptor *opt_desc = sane_get_option_descriptor(handle, i);
        if (opt_desc->type == SANE_TYPE_GROUP) { continue; }

        log->info("[Sane]  -- {:<16}: {:<16} | {:<16} -- {}",
                  opt_desc->name ?: "",
                  enum2str::toStr(opt_desc->type),
                  enum2str::toStr(opt_desc->unit),
                  enum2str::SaneOption_OptionCap_toStr(opt_desc->cap));
        rawOptions.emplace_back(this, i, opt_desc);
    }

    options.refreshFilter();
}

} // namespace qscan::lib
