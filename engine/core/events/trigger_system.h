#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/collision_components.h"
#include "engine/core/events/trigger_components.h"
#include <vector>
#include <string>

namespace urpg {

/**
 * @brief System that detects when entities enter or exit trigger volumes.
 */
class TriggerSystem {
public:
    struct PendingEvent {
        std::string eventId;
        EntityID triggerEntity;
        EntityID activatorEntity;
    };

    void update(World& world) {
        m_pendingEvents.clear();

        // Query all triggers
        world.ForEachWith<TransformComponent, CollisionBoxComponent, TriggerVolumeComponent>([&](EntityID triggerId, TransformComponent& tTrans, CollisionBoxComponent& tBox, TriggerVolumeComponent& trigger) {
            if (!trigger.isActive) return;

            // Ensure trigger has state tracking
            TriggerStateComponent* state = world.GetComponent<TriggerStateComponent>(triggerId);
            if (!state) {
                state = &world.AddComponent(triggerId, TriggerStateComponent{});
            }

            state->wasOccupiedLastFrame = state->isCurrentlyOccupied;
            state->isCurrentlyOccupied = false;

            // Check against all other entities that could activate a trigger
            world.ForEachWith<TransformComponent, CollisionBoxComponent>([&](EntityID actorId, TransformComponent& aTrans, CollisionBoxComponent& aBox) {
                if (triggerId == actorId) return;

                if (checkOverlap(tTrans, tBox, aTrans, aBox)) {
                    state->isCurrentlyOccupied = true;
                    
                    if (!state->wasOccupiedLastFrame) {
                        if (!trigger.onEnterEvent.empty()) {
                            m_pendingEvents.push_back({trigger.onEnterEvent, triggerId, actorId});
                        }
                        if (trigger.isOneShot) {
                            trigger.isActive = false;
                        }
                    }
                }
            });

            if (state->wasOccupiedLastFrame && !state->isCurrentlyOccupied) {
                if (!trigger.onExitEvent.empty()) {
                    m_pendingEvents.push_back({trigger.onExitEvent, triggerId, 0}); // 0 for simplified exit activator
                }
            }
        });
    }

    const std::vector<PendingEvent>& getPendingEvents() const { return m_pendingEvents; }

private:
    bool checkOverlap(const TransformComponent& tTrans, const CollisionBoxComponent& tBox,
                      const TransformComponent& aTrans, const CollisionBoxComponent& aBox) {
        // Simple AABB overlap using existing Vector3 math logic
        Vector3 tMin = tTrans.position + tBox.offset - (tBox.size * Fixed32::FromRaw(32768));
        Vector3 tMax = tTrans.position + tBox.offset + (tBox.size * Fixed32::FromRaw(32768));
        Vector3 aMin = aTrans.position + aBox.offset - (aBox.size * Fixed32::FromRaw(32768));
        Vector3 aMax = aTrans.position + aBox.offset + (aBox.size * Fixed32::FromRaw(32768));

        return (tMin.x < aMax.x && tMax.x > aMin.x) &&
               (tMin.y < aMax.y && tMax.y > aMin.y) &&
               (tMin.z < aMax.z && tMax.z > aMin.z);
    }

    std::vector<PendingEvent> m_pendingEvents;
};

} // namespace urpg
