#include "qscan_log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace {

qscan::lib::logger_t create_logger() {
    qscan::lib::logger_t log = spdlog::stdout_color_mt("qscan");

    log->set_pattern(
        " \x1b[1;35m*\x1b[0m %^%=7l%$ \x1b[1;35m* \x1b[33m[\x1b[0;36m%H:%M:%S.%e\x1b[1;33m]\x1b[0;1m:\x1b[0m %v");

    return log;
}

} // namespace

namespace qscan::lib {

logger_t logger() {
    static logger_t log{create_logger()};
    return log;
}

} // namespace qscan::lib
