#pragma once

#include "SaneDevice.hpp"
#include <functional>
#include <memory>
#include <string>

namespace qscan::lib {

class SaneBackend {

  public:

    struct SaneAuthResult {
        std::string username;
        std::string password;
    };

    using DeviceList = std::vector<std::unique_ptr<SaneDevice>>;

  private:
    int version;

  public:
    explicit SaneBackend(std::function<SaneAuthResult(std::string)> authCallback);
    virtual ~SaneBackend();

    SaneBackend(const SaneBackend &) = delete;
    SaneBackend(SaneBackend &&)      = delete;

    SaneBackend &operator=(const SaneBackend &) = delete;
    SaneBackend &operator=(SaneBackend &&)      = delete;

    [[nodiscard]] DeviceList find_devices();
    [[nodiscard]] int saneVersion();
    [[nodiscard]] std::string saneVersionStr();
};

} // namespace qscan::lib
