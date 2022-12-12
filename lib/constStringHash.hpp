#pragma once

#include <stdint.h>
#include <string>
#include <string_view>

namespace qscan::lib {

const size_t FNV1A_BASE  = 2166136261;
const size_t FNV1A_PRIME = 16777619;

inline size_t fnv1aHash(const char *data) {
    size_t hash = FNV1A_BASE;
    while (*data != 0) {
        hash ^= static_cast<size_t>(*(data++));
        hash *= FNV1A_PRIME;
    }
    return hash;
}

constexpr size_t fnv1aHash(const char *data, size_t n) {
    size_t hash = FNV1A_BASE;
    for (size_t i = 0; i < n; ++i) {
        hash ^= static_cast<size_t>(data[i]);
        hash *= FNV1A_PRIME;
    }
    return hash;
}

size_t fnv1aHash(std::string_view _str) { return fnv1aHash(_str.data(), _str.size()); }

constexpr size_t operator"" _h(const char *data, size_t n) { return fnv1aHash(data, n); }

} // namespace qscan::lib
