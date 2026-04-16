#pragma once

#include "engine/core/math/vector3.h"

namespace urpg {

/**
 * @brief Simple AABB bounding box for collision detection.
 */
struct CollisionBoxComponent {
    Vector3 offset; // Offset from transform position
    Vector3 size = {Fixed32::FromInt(1), Fixed32::FromInt(1), Fixed32::FromInt(1)};
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
