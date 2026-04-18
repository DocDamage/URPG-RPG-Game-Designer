#pragma once

#include <cstdint>

namespace urpg {

/**
 * @brief Simple vector for grid coordinates and pixel offsets.
 */
struct Vector2i {
    int32_t x = 0;
    int32_t y = 0;

    bool operator==(const Vector2i& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2i& other) const {
        return !(*this == other);
    }

    Vector2i operator+(const Vector2i& other) const {
        return { x + other.x, y + other.y };
    }
};

/**
 * @brief Simple floating point vector for positions.
 */
struct Vector2f {
    float x = 0.0f;
    float y = 0.0f;

    Vector2f() = default;
    Vector2f(float _x, float _y) : x(_x), y(_y) {}

    bool operator==(const Vector2f& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2f& other) const {
        return !(*this == other);
    }

    Vector2f operator+(const Vector2f& other) const {
        return { x + other.x, y + other.y };
    }

    Vector2f operator-(const Vector2f& other) const {
        return { x - other.x, y - other.y };
    }
};

/**
 * @brief Direction constants matching MZ order (2=Down, 4=Left, 6=Right, 8=Up).
 */
enum class Direction : uint8_t {
    Down = 2,
    Left = 4,
    Right = 6,
    Up = 8
};

inline Vector2i DirectionToVector(Direction dir) {
    switch (dir) {
        case Direction::Down: return { 0, 1 };
        case Direction::Left: return { -1, 0 };
        case Direction::Right: return { 1, 0 };
        case Direction::Up: return { 0, -1 };
        default: return { 0, 0 };
    }
}

} // namespace urpg
