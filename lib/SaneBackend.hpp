#pragma once

#include "ScanBackend.hpp"

#include <functional>
#include <string>

namespace qscan::lib {

class SaneBackend : public ScanBackend {

  public:

    struct SaneAuthResult {
        std::string username;
        std::string password;
    };

  public:
    explicit SaneBackend(std::function<SaneAuthResult(std::string)> authCallback);
    virtual ~SaneBackend();

    SaneBackend(const SaneBackend &) = delete;
    SaneBackend(SaneBackend &&)      = delete;

    SaneBackend &operator=(const SaneBackend &) = delete;
    SaneBackend &operator=(SaneBackend &&)      = delete;
};

} // namespace qscan::lib
