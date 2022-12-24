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

    [[nodiscard]] const char               *what() const noexcept override;
    [[nodiscard]] inline SANE_Status        saneStatus() const noexcept { return status; }
    [[nodiscard]] inline const std::string &errorMsg() const noexcept { return error; }
};

} // namespace qscan::lib
