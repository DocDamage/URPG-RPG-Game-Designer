#pragma once

#include <cstdint>

namespace urpg {

struct Fixed32 {
    int32_t raw = 0;

    static constexpr Fixed32 FromInt(int32_t v) {
        return Fixed32{v << 16};
    }

    static constexpr Fixed32 FromRaw(int32_t r) {
        return Fixed32{r};
    }

    float ToFloat() const {
        return static_cast<float>(raw) / 65536.0f;
    }

    friend constexpr Fixed32 operator+(Fixed32 a, Fixed32 b) {
        return Fixed32{a.raw + b.raw};
    }

    friend constexpr Fixed32 operator-(Fixed32 a, Fixed32 b) {
        return Fixed32{a.raw - b.raw};
    }

    friend constexpr Fixed32 operator*(Fixed32 a, Fixed32 b) {
        return Fixed32{static_cast<int32_t>((static_cast<int64_t>(a.raw) * static_cast<int64_t>(b.raw)) >> 16)};
    }

    friend constexpr bool operator<(Fixed32 a, Fixed32 b) {
        return a.raw < b.raw;
    }

    friend constexpr bool operator==(Fixed32 a, Fixed32 b) {
        return a.raw == b.raw;
    }
};

} // namespace urpg
