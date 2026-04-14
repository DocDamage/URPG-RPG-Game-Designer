#pragma once

#include "engine/core/math/vector2.h"
#include <vector>
#include <memory>

namespace urpg {

/**
 * @brief Component for entities that can move in world space.
 */
struct MovementComponent {
    Vector2i gridPos;
    Vector2i lastGridPos;
    
    float moveSpeed = 4.0f; // Tiles per second (like MZ speed)
    float moveProgress = 0.0f; // 0.0 to 1.0 between tiles
    
    Direction direction = Direction::Down;
    bool isMoving = false;
};

/**
 * @brief Simple system to update movement offsets and grid transitions.
 */
class MovementSystem {
public:
    /**
     * @brief Updates movement progress based on speed and delta time.
     * @return True if a movement to a new tile was completed this frame.
     */
    static bool Update(MovementComponent& m, float dt) {
        if (!m.isMoving) return false;

        m.moveProgress += m.moveSpeed * dt;

        if (m.moveProgress >= 1.0f) {
            m.moveProgress = 0.0f;
            m.isMoving = false;
            m.lastGridPos = m.gridPos;
            return true;
        }

        return false;
    }

    /**
     * @brief Attempts to start movement to an adjacent cell if not already moving.
     */
    static bool TryMove(MovementComponent& m, Direction dir, const std::function<bool(int, int)>& collisionCheck) {
        if (m.isMoving) return false;

        m.direction = dir;
        Vector2i nextPos = m.gridPos + DirectionToVector(dir);

        if (collisionCheck(nextPos.x, nextPos.y)) {
            return false;
        }

        m.lastGridPos = m.gridPos;
        m.gridPos = nextPos;
        m.isMoving = true;
        m.moveProgress = 0.0f;
        return true;
    }
};

} // namespace urpg
