#pragma once

#include "engine/core/math/vector3.h"

namespace urpg {

/**
 * @brief Simple AABB bounding box for collision detection.
 */
struct CollisionBoxComponent {
    Vector3 offset; // Offset from transform position
    Vector3 size = {Fixed32::FromInt(1), Fixed32::FromInt(1), Fixed32::FromInt(1)};

    CollisionBoxComponent() = default;
    CollisionBoxComponent(Vector3 offsetValue, Vector3 sizeValue)
        : offset(offsetValue), size(sizeValue) {}
    CollisionBoxComponent(Vector3 offsetValue, Fixed32 uniformSize)
        : offset(offsetValue), size{uniformSize, uniformSize, uniformSize} {}
};

/**
 * @brief Result of a collision check.
 */
struct ContactInfo {
    EntityID other;
    Vector3 normal;
    Fixed32 penetration;
};

} // namespace urpg
