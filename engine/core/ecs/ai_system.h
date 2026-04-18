#pragma once

#include <cmath>

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/ai_components.h"
#include "engine/core/ecs/player_control_system.h"

namespace urpg {

/**
 * @brief Simple AI system for actor behaviors (patrolling and basic chasing).
 */
class AISystem {
public:
    void update(World& world, float deltaTime) {
        // Find player position for detection logic
        Vector3 playerPos;
        EntityID playerId = 0;
        world.ForEachWith<TransformComponent, PlayerControlComponent>([&](EntityID id, const TransformComponent& trans, const PlayerControlComponent&) {
            playerPos = trans.position;
            playerId = id;
        });

        world.ForEachWith<TransformComponent, VelocityComponent, AIControlComponent>([&](EntityID id, TransformComponent& transform, VelocityComponent& velocity, AIControlComponent& ai) {
            switch (ai.state) {
                case AIState::Idle:
                    velocity.linear = Vector3::Zero();
                    if (playerId != 0 && isNearby(transform.position, playerPos, ai.detectionRadius)) {
                        ai.state = AIState::Chase;
                    }
                    break;

                case AIState::Patrol:
                    if (ai.patrolPoints.empty()) {
                        ai.state = AIState::Idle;
                        break;
                    }
                    updatePatrol(transform, velocity, ai);
                    if (playerId != 0 && isNearby(transform.position, playerPos, ai.detectionRadius)) {
                        ai.state = AIState::Chase;
                    }
                    break;

                case AIState::Chase:
                    if (playerId == 0 || !isNearby(transform.position, playerPos, ai.detectionRadius * Fixed32::FromInt(2))) {
                        ai.state = AIState::Patrol;
                        break;
                    }
                    moveTowards(transform.position, playerPos, velocity, ai.moveSpeed);
                    break;

                case AIState::Flee:
                    moveTowards(transform.position, playerPos, velocity, ai.moveSpeed * Fixed32::FromInt(-1));
                    break;
            }
        });
    }

private:
    bool isNearby(const Vector3& a, const Vector3& b, Fixed32 radius) {
        Vector3 diff = a - b;
        return (diff.x * diff.x + diff.y * diff.y) <= (radius * radius);
    }

    void moveTowards(const Vector3& current, const Vector3& target, VelocityComponent& velocity, Fixed32 speed) {
        Vector3 diff = target - current;
        // Simplified normalization for demo purposes
        float dx = diff.x.ToFloat();
        float dy = diff.y.ToFloat();
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist > 0.01f) {
            velocity.linear.x = Fixed32::FromRaw(static_cast<int32_t>((dx / dist) * speed.ToFloat() * 65536.0f));
            velocity.linear.y = Fixed32::FromRaw(static_cast<int32_t>((dy / dist) * speed.ToFloat() * 65536.0f));
        } else {
            velocity.linear = Vector3::Zero();
        }
    }

    void updatePatrol(const TransformComponent& transform, VelocityComponent& velocity, AIControlComponent& ai) {
        Vector3 target = ai.patrolPoints[ai.currentPatrolIndex];
        Vector3 diff = target - transform.position;
        
        if (isNearby(transform.position, target, Fixed32::FromRaw(6553))) { // 0.1 units
            ai.currentPatrolIndex = (ai.currentPatrolIndex + 1) % ai.patrolPoints.size();
        } else {
            moveTowards(transform.position, target, velocity, ai.moveSpeed);
        }
    }
};

} // namespace urpg
