#include "engine/core/semver.h"

#include <charconv>
#include <string_view>

namespace urpg {

namespace {

bool ParsePart(std::string_view text, uint16_t& value) {
    if (text.empty()) {
        return false;
    }

    unsigned int parsed = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        return false;
    }
    if (parsed > UINT16_MAX) {
        return false;
    }

    value = static_cast<uint16_t>(parsed);
    return true;
}

} // namespace

bool SemVer::TryParse(const std::string& s, SemVer& out) {
    std::string_view text(s);

    size_t first_dot = text.find('.');
    if (first_dot == std::string_view::npos) {
        return false;
    }

    size_t second_dot = text.find('.', first_dot + 1);
    if (second_dot == std::string_view::npos) {
        return false;
    }

    if (text.find('.', second_dot + 1) != std::string_view::npos) {
        return false;
    }

    uint16_t parsed_major = 0;
    uint16_t parsed_minor = 0;
    uint16_t parsed_patch = 0;

    if (!ParsePart(text.substr(0, first_dot), parsed_major)) {
        return false;
    }
    if (!ParsePart(text.substr(first_dot + 1, second_dot - first_dot - 1), parsed_minor)) {
        return false;
    }
    if (!ParsePart(text.substr(second_dot + 1), parsed_patch)) {
        return false;
    }

    out.major = parsed_major;
    out.minor = parsed_minor;
    out.patch = parsed_patch;
    return true;
}

std::string SemVer::ToString() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

} // namespace urpg
