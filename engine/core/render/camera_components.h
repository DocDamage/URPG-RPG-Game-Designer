#pragma once

#include "engine/core/math/vector3.h"

namespace urpg {

/**
 * @brief Viewport and projection settings for the camera.
 */
struct CameraComponent {
    Fixed32 fov = Fixed32::FromInt(60);
    Fixed32 aspect = Fixed32::FromRaw(65536); // 1.0
    Fixed32 nearPlane = Fixed32::FromRaw(655); // 0.01
    Fixed32 farPlane = Fixed32::FromInt(1000);
    bool isActive = true;
};

} // namespace urpg
