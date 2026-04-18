#pragma once

#include "engine/core/math/vector3.h"
#include "engine/gameplay/combat/combat_calc.h"
#include <string>
#include <vector>
#include <cstdint>

namespace urpg {

/**
 * @brief Component for identifying an entity as an Actor (PC or NPC).
 */
struct ActorComponent {
    std::string name;
    std::string className;
    uint32_t level = 1;
    ActorStats stats;
    bool isEnemy = false;
};

/**
 * @brief Component for velocity for entities that move.
 */
struct VelocityComponent {
    Vector3 linear;
    Vector3 angular;
};

/**
 * @brief Component for position on the map (tiles or world-space).
 */
struct TransformComponent {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale = {Fixed32::FromInt(1), Fixed32::FromInt(1), Fixed32::FromInt(1)};
};

/**
 * @brief Component for a sprite or visual representation.
 */
struct VisualComponent {
    std::string assetPath;
    int32_t frameIndex = 0;
    bool visible = true;
};

} // namespace urpg
