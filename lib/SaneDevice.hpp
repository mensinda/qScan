#pragma once

#include "SaneImage.hpp"
#include "SaneOption.hpp"
#include "SaneOptionsWrapper.hpp"
#include <sane/sane.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace qscan::lib {

class SaneDevice {
  public:
    using snapshot_t = std::unordered_map<std::string, SaneOption::value_t>;

  private:
    std::string name;
    std::string vendor;
    std::string model;
    std::string type;

    SANE_Handle handle = nullptr;

    std::vector<SaneOption> rawOptions;

    SaneOptionsWrapper options;

    size_t expectedBytes = 0;
    size_t readBytes     = 0;

    bool disableOptionReload = false;

  public:

    explicit SaneDevice(const SANE_Device *device);
    virtual ~SaneDevice();

    void connect();
    void reload_options();

    [[nodiscard]] const std::string  &getName() const { return name; }
    [[nodiscard]] const std::string  &getVendor() const { return vendor; }
    [[nodiscard]] const std::string  &getModel() const { return model; }
    [[nodiscard]] const std::string  &getType() const { return type; }
    [[nodiscard]] SaneOptionsWrapper &getOptions() { return options; }
    [[nodiscard]] SANE_Handle         getHandle() { return handle; }

    [[nodiscard]] SaneImage scan();
    [[nodiscard]] double    scanProgress() const;

    void cancelScan();

    [[nodiscard]] snapshot_t optionsSnapshot() const;

    void applyOptionSnapshot(const snapshot_t &_snapshot);

    // No copy or move

    SaneDevice(const SaneDevice &) = delete;
    SaneDevice(SaneDevice &&)      = delete;

    SaneDevice &operator=(const SaneDevice &) = delete;
    SaneDevice &operator=(SaneDevice &&)      = delete;
};

} // namespace qscan::lib
