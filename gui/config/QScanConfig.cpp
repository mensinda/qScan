#include "QScanConfig.hpp"

#include "qscan_log.hpp"
#include "util/constStringHash.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include <QStandardPaths>

using json   = nlohmann::json;
namespace fs = std::filesystem;

using namespace qscan::lib;

namespace {

fs::path cfgFile() {
    fs::path cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString();
    if (!is_directory(cfgDir)) {
        create_directories(cfgDir);
    }
    return cfgDir / "config.json";
}

} // namespace

namespace qscan::gui {

void to_json(json &j, const QScanConfig::LastDevice &cfg) {
    json opts{};
    for (const auto &rawOpt : cfg.options) {
        SaneOption::value_t value = rawOpt.second;
        switch (value.index()) {
            case 0:
                opts[rawOpt.first] = {
                    {"type",  "bool"               },
                    {"value", std::get<bool>(value)},
                };
                break;
            case 1:
                opts[rawOpt.first] = {
                    {"type",  "int"               },
                    {"value", std::get<int>(value)},
                };
                break;
            case 2:
                opts[rawOpt.first] = {
                    {"type",  "double"               },
                    {"value", std::get<double>(value)},
                };
                break;
            case 3:
                opts[rawOpt.first] = {
                    {"type",  "string"                    },
                    {"value", std::get<std::string>(value)},
                };
                break;
            default: throw std::runtime_error("Unknown option variant index");
        }
    }

    j = json{
        {"name",    cfg.name  },
        {"vendor",  cfg.vendor},
        {"model",   cfg.model },
        {"type",    cfg.type  },
        {"options", opts      },
    };
}


void from_json(const json &j, QScanConfig::LastDevice &cfg) {
    cfg.name   = j.value("name", "");
    cfg.vendor = j.value("vendor", "");
    cfg.model  = j.value("model", "");
    cfg.type   = j.value("type", "");

    json options{};
    if (j.contains("options")) {
        options = j["options"];
    }

    cfg.options.clear();
    for (const auto &[key, value] : options.items()) {
        SaneOption::value_t val;
        switch (fnv1aHash(value["type"].get<std::string>())) {
            case "bool"_h: val = value["value"].get<bool>(); break;
            case "int"_h: val = value["value"].get<int>(); break;
            case "double"_h: val = value["value"].get<double>(); break;
            case "string"_h: val = value["value"].get<std::string>(); break;
            default: throw std::runtime_error("Unknown type: " + value["type"].get<std::string>());
        }
        cfg.options[key] = val;
    }
}

void to_json(json &j, const QScanConfig::Window &window) {
    j = {
        {"state",    window.state.toBase64()   },
        {"geometry", window.geometry.toBase64()},
    };
}

void from_json(const json &j, QScanConfig::Window &cfg) {
    cfg.state    = QByteArray::fromBase64(QByteArray::fromStdString(j.value("state", "")));
    cfg.geometry = QByteArray::fromBase64(QByteArray::fromStdString(j.value("geometry", "")));
}

void to_json(json &j, const QScanConfig::BatchScanning &cfg) {
    j = {
        {"max",   cfg.max  },
        {"delay", cfg.delay},
    };
}

void from_json(const json &j, QScanConfig::BatchScanning &cfg) {
    cfg.max   = j.value("max", -10);
    cfg.delay = j.value("delay", -10);
}

void to_json(json &j, const QScanConfig &cfg) {
    j = json{
        {"window", cfg.window},
        {"batch",  cfg.batch },
    };

    if (cfg.lastDevice) {
        j["lastDevice"] = *cfg.lastDevice;
    }
}

void from_json(const json &j, QScanConfig &cfg) {
    cfg.lastDevice = {};
    cfg.window     = {};
    cfg.batch      = {};
    if (j.contains("lastDevice")) {
        cfg.lastDevice = j["lastDevice"].get<QScanConfig::LastDevice>();
    }
    if (j.contains("window")) {
        cfg.window = j["window"].get<QScanConfig::Window>();
    }
    if (j.contains("batch")) {
        cfg.batch = j["batch"].get<QScanConfig::BatchScanning>();
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
    if (!ifs.is_open()) {
        return;
    }
    json j;
    ifs >> j;
    from_json(j, *this);
}


} // namespace qscan::gui
