#pragma once

#include "SaneDevice.hpp"
#include "SaneOption.hpp"

#include <optional>
#include <string>
#include <unordered_map>

#include <QByteArray>

namespace qscan::gui {

struct QScanConfig {

    struct LastDevice {
        std::string name;
        std::string vendor;
        std::string model;
        std::string type;

        lib::SaneDevice::snapshot_t options;
    };

    struct Window {
        QByteArray state;
        QByteArray geometry;
    };

    struct BatchScanning {
        int max   = -11;
        int delay = -11;
    };

    std::optional<LastDevice> lastDevice;
    Window                    window;
    BatchScanning             batch;

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
