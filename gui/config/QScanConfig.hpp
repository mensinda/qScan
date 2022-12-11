#pragma once

#include <optional>
#include <string>

namespace qscan::gui {

struct QScanConfig {

    struct LastDevice {
        std::string name;
        std::string vendor;
        std::string model;
        std::string type;
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
