#include "QScanConfig.hpp"
#include "qscan_log.hpp"

#include <nlohmann/json.hpp>
#include <QStandardPaths>
#include <filesystem>
#include <fstream>

using json   = nlohmann::json;
namespace fs = std::filesystem;

using namespace qscan::lib;

namespace {

fs::path cfgFile() {
    fs::path cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString();
    if (!is_directory(cfgDir)) { create_directories(cfgDir); }
    return cfgDir / "config.json";
}

} // namespace

namespace qscan::gui {

void to_json(json &j, const QScanConfig::LastDevice &cfg) {
    j = json{
        {"name",   cfg.name  },
        {"vendor", cfg.vendor},
        {"model",  cfg.model },
        {"type",   cfg.type  },
    };
}

void from_json(const json &j, QScanConfig::LastDevice &cfg) {
    cfg.name   = j.value("name", "");
    cfg.vendor = j.value("vendor", "");
    cfg.model  = j.value("model", "");
    cfg.type   = j.value("type", "");
}

void to_json(json &j, const QScanConfig &cfg) {
    j = json{};

    if (cfg.lastDevice) { j["lastDevice"] = *cfg.lastDevice; }
}

void from_json(const json &j, QScanConfig &cfg) {
    if (j.contains("lastDevice")) {
        cfg.lastDevice = j["lastDevice"].get<QScanConfig::LastDevice>();
    } else {
        cfg.lastDevice = {};
    }
}

QScanConfig::QScanConfig() { load(); }
QScanConfig::~QScanConfig() { save(); }

void QScanConfig::save() {
    json j = *this;

    std::ofstream ofs(cfgFile());
    ofs << j.dump(2) << std::endl;
}

void QScanConfig::load() {
    std::ifstream ifs(cfgFile());
    json j;
    ifs >> j;
    from_json(j, *this);
}


} // namespace qscan::gui
