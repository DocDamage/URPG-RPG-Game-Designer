#pragma once

#include "engine/core/math/fixed32.h"
#include <string>
#include <vector>

namespace urpg {

/**
 * @brief States for a pathfollowing entity.
 */
enum class AIState : uint8_t {
    Idle,
    Patrol,
    Chase,
    Flee
};

/**
 * @brief Component that drives an entity's movement via AI logic.
 */
struct AIControlComponent {
    AIState state = AIState::Idle;
    std::vector<Vector3> patrolPoints;
    size_t currentPatrolIndex = 0;
    EntityID currentTarget = 0;
    Fixed32 detectionRadius = Fixed32::FromInt(10);
    Fixed32 moveSpeed = Fixed32::FromInt(3);
};

} // namespace urpg
