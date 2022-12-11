#pragma once

#include "SaneOption.hpp"
#include <sane/sane.h>
#include <string>
#include <vector>

namespace qscan::lib {

class SaneDevice {

  private:
    std::string name;
    std::string vendor;
    std::string model;
    std::string type;

    SANE_Handle handle = nullptr;

    std::vector<SaneOption> options;

  public:

    explicit SaneDevice(const SANE_Device *device);
    virtual ~SaneDevice();

    void connect();
    void reload_options();

    SaneDevice(const SaneDevice &) = delete;
    SaneDevice(SaneDevice &&)      = delete;

    SaneDevice &operator=(const SaneDevice &) = delete;
    SaneDevice &operator=(SaneDevice &&)      = delete;

    [[nodiscard]] const std::string             &getName() const { return name; }
    [[nodiscard]] const std::string             &getVendor() const { return vendor; }
    [[nodiscard]] const std::string             &getModel() const { return model; }
    [[nodiscard]] const std::string             &getType() const { return type; }
    [[nodiscard]] const std::vector<SaneOption> &getOptions() const { return options; }
    [[nodiscard]] SANE_Handle                    getHandle() { return handle; }
};

} // namespace qscan::lib
