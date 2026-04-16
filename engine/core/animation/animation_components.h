#pragma once

#include "engine/core/math/vector3.h"
#include <vector>

namespace urpg {

/**
 * @brief Represents a single keyframe in a property animation track.
 */
struct AnimationKeyframe {
    Fixed32 time;
    Vector3 value;
};

/**
 * @brief Component that drives property changes over time.
 */
struct AnimationComponent {
    std::vector<AnimationKeyframe> positionTrack;
    std::vector<AnimationKeyframe> rotationTrack;
    Fixed32 currentTime = Fixed32::FromInt(0);
    Fixed32 duration = Fixed32::FromInt(0);
    bool isLooping = true;
    bool isPlaying = true;
};

} // namespace urpg
