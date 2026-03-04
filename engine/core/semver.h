#pragma once

#include <cstdint>
#include <string>

namespace urpg {

struct SemVer {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;

    static bool TryParse(const std::string& s, SemVer& out);
    std::string ToString() const;
};

inline bool operator==(const SemVer& a, const SemVer& b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

inline bool operator<(const SemVer& a, const SemVer& b) {
    if (a.major != b.major) {
        return a.major < b.major;
    }
    if (a.minor != b.minor) {
        return a.minor < b.minor;
    }
    return a.patch < b.patch;
}

} // namespace urpg
