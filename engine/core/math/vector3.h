#pragma once

#include "fixed32.h"

namespace urpg {

struct Vector3 {
    Fixed32 x;
    Fixed32 y;
    Fixed32 z;

    static constexpr Vector3 Zero() {
        return Vector3{Fixed32::FromRaw(0), Fixed32::FromRaw(0), Fixed32::FromRaw(0)};
    }

    friend constexpr Vector3 operator+(const Vector3& a, const Vector3& b) {
        return Vector3{a.x + b.x, a.y + b.y, a.z + b.z};
    }

    friend constexpr Vector3 operator-(const Vector3& a, const Vector3& b) {
        return Vector3{a.x - b.x, a.y - b.y, a.z - b.z};
    }

    friend constexpr Vector3 operator*(const Vector3& a, Fixed32 scalar) {
        return Vector3{a.x * scalar, a.y * scalar, a.z * scalar};
    }

    friend constexpr bool operator==(const Vector3& a, const Vector3& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    friend constexpr bool operator!=(const Vector3& a, const Vector3& b) {
        return !(a == b);
    }
};

} // namespace urpg
