#include "SaneBackend.hpp"

#include <cstring>
#include <sane/sane.h>
#include <stdexcept>

#include "enum2str.hpp"
#include "qscan_log.hpp"

namespace qscan::lib {

SaneBackend::SaneBackend(std::function<SaneAuthResult(std::string)> authCallback) {
    static auto staticAuthCallback = std::move(authCallback);
    int         version;

    SANE_Status status = sane_init(&version, [](SANE_String_Const resource, SANE_Char *username, SANE_Char *password) {
        auto [username_str, password_str] = staticAuthCallback(std::string{resource});

        std::strncpy(username, username_str.c_str(), SANE_MAX_USERNAME_LEN);
        std::strncpy(password, password_str.c_str(), SANE_MAX_PASSWORD_LEN);
    });

    if (status != SANE_STATUS_GOOD) {
        throw std::runtime_error(fmt::format("sane_init failed with {}", enum2str::toStr(status)));
    }

    logger()->info("[Sane] initialized (version = {}.{}.{})",
                   SANE_VERSION_MAJOR(version),
                   SANE_VERSION_MINOR(version),
                   SANE_VERSION_BUILD(version));
}

SaneBackend::~SaneBackend() {
    sane_exit();
    logger()->info("[Sane] terminated");
}

} // namespace qscan::lib