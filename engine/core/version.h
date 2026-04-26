#pragma once

#include <urpg_version.h>

namespace urpg {

inline constexpr int VersionMajor = URPG_VERSION_MAJOR;
inline constexpr int VersionMinor = URPG_VERSION_MINOR;
inline constexpr int VersionPatch = URPG_VERSION_PATCH;
inline constexpr const char* VersionString = URPG_VERSION_STRING;

inline constexpr const char* versionString() {
    return VersionString;
}

} // namespace urpg
