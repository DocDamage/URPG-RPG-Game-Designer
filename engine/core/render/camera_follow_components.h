#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Logic for how the camera follows a target.
 */
enum class CameraFollowMode : uint8_t {
    Static,   // Fixed position
    Simple,   // Snap to target
    Smooth,   // Interpolate towards target
    Delayed   // Follow with a small delay or "dead zone"
};

/**
 * @brief Component that makes a camera entity follow another entity.
 */
struct CameraFollowComponent {
    EntityID target = 0;
    CameraFollowMode mode = CameraFollowMode::Smooth;
    Vector3 offset = {Fixed32::FromInt(0), Fixed32::FromInt(0), Fixed32::FromInt(-10)};
    Fixed32 smoothFactor = Fixed32::FromRaw(6553); // ~0.1
};

} // namespace urpg
