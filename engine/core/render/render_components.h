#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Configuration for a single frame of an animation.
 */
struct AnimationFrame {
    int textureIndex;
    Fixed32 duration; // How long to stay on this frame
};

/**
 * @brief A sequence of frames defining an animation clip (e.g., "Walk", "Attack").
 */
struct AnimationClip {
    std::string name;
    std::vector<AnimationFrame> frames;
    bool loop = true;
};

/**
 * @brief Component attached to entities that can play animations.
 */
struct RenderAnimationComponent {
    std::string currentClip;
    int currentFrameIndex = 0;
    Fixed32 elapsedTime;
    bool isPlaying = true;
};

/**
 * @brief Data describing how to render a static or animated sprite.
 */
struct SpriteComponent {
    std::string textureId;
    int srcX = 0;
    int srcY = 0;
    int width = 32;
    int height = 32;
    bool flipX = false;
    bool visible = true;
};

} // namespace urpg
