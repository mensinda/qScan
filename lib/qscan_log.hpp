#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace qscan::lib {

using logger_t = std::shared_ptr<spdlog::logger>;

[[nodiscard]] logger_t logger();

} // namespace qscan::lib
