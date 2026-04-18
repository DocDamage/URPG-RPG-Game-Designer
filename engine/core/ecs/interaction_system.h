#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/gameplay_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/input/input_core.h"
#include <vector>
#include <string>

namespace urpg {

/**
 * @brief System that handles player interaction with the world.
 * Detects nearby interactable entities when the 'Confirm' action is triggered.
 */
class InteractionSystem {
public:
    struct InteractionResult {
        EntityID target;
        std::string eventId;
    };

    void update(World& world, const input::InputCore& input) {
        m_pendingInteractions.clear();

        if (!input.isActionJustPressed(input::InputAction::Confirm)) {
            return;
        }

        // Find the player entity (assuming one entity with PlayerControlComponent)
        EntityID playerId = 0;
        Vector3 playerPos = Vector3::Zero();
        world.ForEachWith<TransformComponent, PlayerControlComponent>([&](EntityID id, const TransformComponent& trans, const PlayerControlComponent&) {
            playerId = id;
            playerPos = trans.position;
        });

        // Find the closest interactable entity
        EntityID bestTarget = 0;
        std::string bestEvent;
        float bestDistSq = 99999.0f;

        world.ForEachWith<TransformComponent, InteractionComponent>([&](EntityID id, const TransformComponent& trans, const InteractionComponent& interaction) {
            if (id == playerId) return;

            Vector3 diff = trans.position - playerPos;
            const float dx = diff.x.ToFloat();
            const float dy = diff.y.ToFloat();
            const float distSq = dx * dx + dy * dy;
            const float radius = interaction.interactionRadius.ToFloat();
            const float radiusSq = radius * radius;

            if (distSq <= radiusSq && distSq < bestDistSq) {
                bestDistSq = distSq;
                bestTarget = id;
                bestEvent = interaction.interactionEvent;
            }
        });

        if (bestTarget != 0) {
            m_pendingInteractions.push_back({bestTarget, bestEvent});
        }
    }

    const std::vector<InteractionResult>& getPendingInteractions() const { return m_pendingInteractions; }

private:
    std::vector<InteractionResult> m_pendingInteractions;
};

} // namespace urpg
