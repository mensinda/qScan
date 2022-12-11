#pragma once

#include <exception>
#include <sane/sane.h>
#include <string>

namespace qscan::lib {

class SaneException : public std::exception {

  private:
    SANE_Status status;
    std::string error;

  public:
    explicit SaneException(SANE_Status _status, std::string _errorText);
    ~SaneException() override = default;

    SaneException(const SaneException &) = default;
    SaneException(SaneException &&)      = default;

    SaneException &operator=(const SaneException &) = default;
    SaneException &operator=(SaneException &&)      = default;

    const char *what() const noexcept override;
};

} // namespace qscan::lib
