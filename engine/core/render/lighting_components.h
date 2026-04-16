#pragma once

#include "engine/core/math/vector3.h"
#include "engine/core/ecs/world.h"

namespace urpg {

/**
 * @brief Component representing a point light source in the world.
 */
struct PointLightComponent {
    Vector3 color = {Fixed32::FromInt(1), Fixed32::FromInt(1), Fixed32::FromInt(1)}; // RGB 0-1
    Fixed32 intensity = Fixed32::FromInt(1);
    Fixed32 radius = Fixed32::FromInt(10);
};

/**
 * @brief Global ambient light settings for a scene.
 */
struct AmbientLightComponent {
    Vector3 color = {Fixed32::FromInt(0), Fixed32::FromInt(0), Fixed32::FromInt(0)};
    Fixed32 intensity = Fixed32::FromInt(0);
};

} // namespace urpg
