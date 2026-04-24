#include "window_color_parser.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string_view>
#include <variant>

namespace urpg {
namespace compat {

namespace {

uint8_t clampColorByte(int64_t value) {
    return static_cast<uint8_t>(std::clamp<int64_t>(value, 0, 255));
}

uint8_t lerpColorByte(uint8_t from, uint8_t to, double t) {
    const double blended =
        static_cast<double>(from) + (static_cast<double>(to) - static_cast<double>(from)) * t;
    return clampColorByte(static_cast<int64_t>(std::lround(blended)));
}

std::optional<int64_t> numericValueAsInt(const Value& value) {
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return *integer;
    }
    if (const auto* number = std::get_if<double>(&value.v)) {
        return static_cast<int64_t>(std::lround(*number));
    }
    return std::nullopt;
}

std::optional<Color> colorFromObject(const Object& object) {
    const auto lookupByte = [&](const std::string& key, uint8_t fallback) -> uint8_t {
        const auto it = object.find(key);
        if (it == object.end()) {
            return fallback;
        }
        const auto value = numericValueAsInt(it->second);
        return value.has_value() ? clampColorByte(*value) : fallback;
    };

    const bool hasShortRgb =
        object.find("r") != object.end() ||
        object.find("g") != object.end() ||
        object.find("b") != object.end();
    const bool hasLongRgb =
        object.find("red") != object.end() ||
        object.find("green") != object.end() ||
        object.find("blue") != object.end();
    if (!hasShortRgb && !hasLongRgb) {
        return std::nullopt;
    }

    return Color{
        lookupByte("r", lookupByte("red", 0)),
        lookupByte("g", lookupByte("green", 0)),
        lookupByte("b", lookupByte("blue", 0)),
        lookupByte("a", lookupByte("alpha", 255)),
    };
}

std::optional<Color> colorFromArray(const Array& array) {
    if (array.size() < 3) {
        return std::nullopt;
    }

    const auto channel = [&](std::size_t index, uint8_t fallback) -> uint8_t {
        if (index >= array.size()) {
            return fallback;
        }
        const auto value = numericValueAsInt(array[index]);
        return value.has_value() ? clampColorByte(*value) : fallback;
    };

    return Color{
        channel(0, 0),
        channel(1, 0),
        channel(2, 0),
        channel(3, 255),
    };
}

int hexValue(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }
    if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    }
    return -1;
}

std::optional<Color> colorFromHexString(std::string_view text) {
    if (!text.empty() && text.front() == '#') {
        text.remove_prefix(1);
    } else if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        text.remove_prefix(2);
    }

    if (text.size() != 6 && text.size() != 8) {
        return std::nullopt;
    }

    uint32_t value = 0;
    for (const char ch : text) {
        const int digit = hexValue(ch);
        if (digit < 0) {
            return std::nullopt;
        }
        value = (value << 4) | static_cast<uint32_t>(digit);
    }

    if (text.size() == 6) {
        return Color{
            static_cast<uint8_t>((value >> 16) & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>(value & 0xFF),
            255,
        };
    }

    return Color::fromHex(value);
}

std::optional<Color> colorFromRgbFunctionString(const std::string& text) {
    const bool isRgb = text.rfind("rgb(", 0) == 0;
    const bool isRgba = text.rfind("rgba(", 0) == 0;
    if (!isRgb && !isRgba) {
        return std::nullopt;
    }
    if (text.empty() || text.back() != ')') {
        return std::nullopt;
    }

    const std::size_t open = text.find('(');
    std::string payload = text.substr(open + 1, text.size() - open - 2);
    std::replace(payload.begin(), payload.end(), ',', ' ');

    std::istringstream stream(payload);
    int64_t r = 0;
    int64_t g = 0;
    int64_t b = 0;
    double alpha = 255.0;
    if (!(stream >> r >> g >> b)) {
        return std::nullopt;
    }
    if (stream >> alpha && alpha >= 0.0 && alpha <= 1.0) {
        alpha *= 255.0;
    }

    return Color{
        clampColorByte(r),
        clampColorByte(g),
        clampColorByte(b),
        clampColorByte(static_cast<int64_t>(std::lround(alpha))),
    };
}

std::optional<Color> colorFromString(const std::string& text) {
    if (auto color = colorFromHexString(text)) {
        return color;
    }
    return colorFromRgbFunctionString(text);
}

} // namespace

Color lerpColor(const Color& from, const Color& to, double t) {
    return Color{
        lerpColorByte(from.r, to.r, t),
        lerpColorByte(from.g, to.g, t),
        lerpColorByte(from.b, to.b, t),
        lerpColorByte(from.a, to.a, t),
    };
}

std::optional<Color> colorFromValue(const Value& value, const Window_Base* windowForSystemColor) {
    if (const auto integer = numericValueAsInt(value)) {
        if (*integer >= 0 && *integer <= 31 && windowForSystemColor != nullptr) {
            return windowForSystemColor->systemColor(static_cast<int32_t>(*integer));
        }
        if (*integer >= 0 && *integer <= 0xFFFFFF) {
            const auto packed = static_cast<uint32_t>(*integer);
            return Color{
                static_cast<uint8_t>((packed >> 16) & 0xFF),
                static_cast<uint8_t>((packed >> 8) & 0xFF),
                static_cast<uint8_t>(packed & 0xFF),
                255,
            };
        }
        if (*integer >= 0 && *integer <= 0xFFFFFFFFLL) {
            return Color::fromHex(static_cast<uint32_t>(*integer));
        }
    }
    if (const auto* object = std::get_if<Object>(&value.v)) {
        return colorFromObject(*object);
    }
    if (const auto* array = std::get_if<Array>(&value.v)) {
        return colorFromArray(*array);
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        return colorFromString(*text);
    }
    return std::nullopt;
}

} // namespace compat
} // namespace urpg
