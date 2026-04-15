#pragma once

#include <memory>
#include <string>
#include "engine/core/platform/gl_texture.h"
#include "engine/core/sprite_batcher.h"
#include "engine/core/math/vector2.h"

namespace urpg {

/**
 * @brief Handles rendering of character sprites with frame-based animation.
 * Optimized for RPG Maker MZ/MV character sheets (3x4 or 12x8 layouts).
 */
class SpriteAnimator {
public:
    struct AnimationConfig {
        int framesX = 3;      // Frames per row
        int framesY = 4;      // Frames per column (directions)
        float frameDuration = 0.15f; // Time per frame in seconds
    };

    SpriteAnimator(const std::shared_ptr<Texture>& texture);
    SpriteAnimator(const std::shared_ptr<Texture>& texture, const AnimationConfig& config);
    ~SpriteAnimator() = default;

    /**
     * @brief Update the animation state.
     * @param deltaTime Time since last frame.
     */
    void update(float deltaTime);

    /**
     * @brief Set the current animation row (usually direction).
     */
    void setRow(int row);

    /**
     * @brief Shortcut to set row based on engine Direction.
     */
    void setDirection(Direction dir) {
        switch (dir) {
            case Direction::Down:  setRow(0); break;
            case Direction::Left:  setRow(1); break;
            case Direction::Right: setRow(2); break;
            case Direction::Up:    setRow(3); break;
        }
    }

    /**
     * @brief Set whether the character is currently walking (animating).
     */
    void setMoving(bool moving) { m_isWalking = moving; }

    /**
     * @brief Reset animation to the first frame.
     */
    void reset();

    /**
     * @brief Submits the current frame to the batcher.
     */
    void draw(SpriteBatcher& batcher, float x, float y, float w, float h, float z = 0.0f);

    /**
     * @brief Get the current texture.
     */
    std::shared_ptr<Texture> getTexture() const { return m_texture; }

private:
    std::shared_ptr<Texture> m_texture;
    AnimationConfig m_config;
    
    int m_currentRow = 0;
    int m_currentFrame = 0;
    float m_elapsedTime = 0.0f;

    bool m_isLooping = true;
    bool m_isWalking = false;
};

} // namespace urpg
