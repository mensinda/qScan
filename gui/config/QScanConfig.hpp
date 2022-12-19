#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include "SaneOption.hpp"

namespace qscan::gui {

struct QScanConfig {

    struct LastDevice {
        std::string name;
        std::string vendor;
        std::string model;
        std::string type;

        std::unordered_map<std::string, lib::SaneOption::value_t> options;
    };

    std::optional<LastDevice> lastDevice;

    void save();
    void load();

    // Housekeeping...

    QScanConfig();
    ~QScanConfig();

    QScanConfig(const QScanConfig &) = delete;
    QScanConfig(QScanConfig &&)      = delete;

    QScanConfig &operator=(const QScanConfig &) = delete;
    QScanConfig &operator=(QScanConfig &&)      = delete;
};

} // namespace qscan::gui
