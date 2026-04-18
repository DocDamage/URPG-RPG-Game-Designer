#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Logic for how the camera shakes.
 */
enum class CameraShakeMode : uint8_t {
    Rotational, // Shakes by rotating slightly around the view axis
    Positional, // Shakes by offsetting the position
    Both        // Combined rotational and positional shake
};

/**
 * @brief Component that applies procedural trauma/shake effects to a camera.
 */
struct CameraShakeComponent {
    Fixed32 trauma = Fixed32::FromInt(0);       // Normalized trauma [0, 1]
    Fixed32 traumaDecay = Fixed32::FromInt(1);  // Trauma lost per second
    Fixed32 maxTranslation = Fixed32::FromInt(2); // Max units of movement
    Fixed32 maxRotation = Fixed32::FromInt(5);    // Max degrees of rotation
    CameraShakeMode mode = CameraShakeMode::Both;
};

} // namespace urpg
