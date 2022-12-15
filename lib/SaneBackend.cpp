#include "SaneBackend.hpp"

#include <cstring>
#include <sane/sane.h>
#include <stdexcept>

#include "SaneException.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"

namespace qscan::lib {

SaneBackend::SaneBackend(std::function<SaneAuthResult(std::string)> authCallback) {
    static auto staticAuthCallback = std::move(authCallback);

    SANE_Status status = sane_init(&version, [](SANE_String_Const resource, SANE_Char *username, SANE_Char *password) {
        auto [username_str, password_str] = staticAuthCallback(std::string{resource});

        std::strncpy(username, username_str.c_str(), SANE_MAX_USERNAME_LEN);
        std::strncpy(password, password_str.c_str(), SANE_MAX_PASSWORD_LEN);
    });

    if (status != SANE_STATUS_GOOD) {
        throw SaneException(status, "sane_init failed");
    }

    logger()->info("[Sane] initialized (version = {})", saneVersionStr());
}

SaneBackend::~SaneBackend() {
    sane_exit();
    logger()->info("[Sane] terminated");
}

SaneBackend::DeviceList SaneBackend::find_devices() {
    logger_t log = logger();

    std::vector<std::unique_ptr<SaneDevice>> res{};

    log->info("[Sane] Start scanning for sane devices");

    const SANE_Device **sane_device_list;
    const SANE_Status   status = sane_get_devices(&sane_device_list, SANE_FALSE);

    if (status != SANE_STATUS_GOOD) {
        throw SaneException(status, "Failed to get sane devices");
    }

    const SANE_Device *sane_device = *sane_device_list;

    while (sane_device != nullptr) {
        log->info("[Sane]   -- Found {} (type = {}; model = {}; vendor = {})",
                  sane_device->name,
                  sane_device->type,
                  sane_device->model,
                  sane_device->vendor);

        res.emplace_back(new SaneDevice(sane_device));
        sane_device = *(++sane_device_list);
    }

    log->info("[Sane] Found {} devices", res.size());

    return res;
}

int SaneBackend::saneVersion() { return version; }

std::string SaneBackend::saneVersionStr() {
    return fmt::format("{}.{}.{}",
                       SANE_VERSION_MAJOR(version),
                       SANE_VERSION_MINOR(version),
                       SANE_VERSION_BUILD(version));
}

std::string SaneBackend::saneUnitToDisplayString(SANE_Unit unit) {
    switch (unit) {
        case SANE_UNIT_BIT: return "bit";
        case SANE_UNIT_DPI: return "DPI";
        case SANE_UNIT_MICROSECOND: return "μs";
        case SANE_UNIT_MM: return "mm";
        case SANE_UNIT_PERCENT: return "%";
        case SANE_UNIT_PIXEL: return "px";
        case SANE_UNIT_NONE: return "";
        default: return "UNKNOWN";
    }
}

} // namespace qscan::lib