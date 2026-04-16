#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/gameplay_components.h"
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

        if (!input.isActionActive(input::InputAction::Confirm)) {
            m_wasConfirmPressed = false;
            return;
        }

        // Only trigger on initial press
        if (m_wasConfirmPressed) return;
        m_wasConfirmPressed = true;

        // Find the player entity (assuming one entity with PlayerControlComponent)
        EntityID playerId = 0;
        Vector3 playerPos;
        world.ForEachWith<TransformComponent, PlayerControlComponent>([&](EntityID id, const TransformComponent& trans, const PlayerControlComponent&) {
            playerId = id;
            playerPos = trans.position;
        });

        if (playerId == 0) return;

        // Find the closest interactable entity
        EntityID bestTarget = 0;
        std::string bestEvent;
        Fixed32 bestDistSq = Fixed32::FromInt(99999);

        world.ForEachWith<TransformComponent, InteractionComponent>([&](EntityID id, const TransformComponent& trans, const InteractionComponent& interaction) {
            if (id == playerId) return;

            Vector3 diff = trans.position - playerPos;
            // Simple distance check in 2D (ignoring Z for typical RPG interaction)
            Fixed32 distSq = (diff.x * diff.x) + (diff.y * diff.y);
            Fixed32 radiusSq = interaction.interactionRadius * interaction.interactionRadius;

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
    bool m_wasConfirmPressed = false;
    std::vector<InteractionResult> m_pendingInteractions;
};

} // namespace urpg
