#include "SaneException.hpp"

#include "enum2str.hpp"
#include <spdlog/spdlog.h>

namespace qscan::lib {

SaneException::SaneException(SANE_Status _status, std::string _errorText) : status(_status) {
    error = fmt::format("{}: {}", _errorText, enum2str::toStr(status));
}

const char *SaneException::what() const noexcept { return error.c_str(); }

} // namespace qscan::lib
