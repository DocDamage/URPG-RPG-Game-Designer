#pragma once

#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/collision_components.h"
#include "engine/core/ecs/world.h"
#include "engine/core/events/trigger_components.h"
#include <string>
#include <vector>

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
        world.ForEachWith<TransformComponent, CollisionBoxComponent, TriggerVolumeComponent>(
            [&](EntityID triggerId, TransformComponent& tTrans, CollisionBoxComponent& tBox,
                TriggerVolumeComponent& trigger) {
                if (!trigger.isActive)
                    return;

                // Ensure trigger has state tracking
                TriggerStateComponent* state = world.GetComponent<TriggerStateComponent>(triggerId);
                if (!state) {
                    state = &world.AddComponent(triggerId, TriggerStateComponent{});
                }

                state->wasOccupiedLastFrame = state->isCurrentlyOccupied;
                state->isCurrentlyOccupied = false;

                // Check against all other entities that could activate a trigger
                world.ForEachWith<TransformComponent, CollisionBoxComponent>(
                    [&](EntityID actorId, TransformComponent& aTrans, CollisionBoxComponent& aBox) {
                        if (triggerId == actorId)
                            return;

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
                        m_pendingEvents.push_back(
                            {trigger.onExitEvent, triggerId, 0}); // 0 for simplified exit activator
                    }
                }
            });
    }

    const std::vector<PendingEvent>& getPendingEvents() const { return m_pendingEvents; }

  private:
    bool checkOverlap(const TransformComponent& tTrans, const CollisionBoxComponent& tBox,
                      const TransformComponent& aTrans, const CollisionBoxComponent& aBox) {
        const float tHalfX = tBox.size.x.ToFloat() * 0.5f;
        const float tHalfY = tBox.size.y.ToFloat() * 0.5f;
        const float tHalfZ = tBox.size.z.ToFloat() * 0.5f;
        const float aHalfX = aBox.size.x.ToFloat() * 0.5f;
        const float aHalfY = aBox.size.y.ToFloat() * 0.5f;
        const float aHalfZ = aBox.size.z.ToFloat() * 0.5f;

        const float tCenterX = (tTrans.position + tBox.offset).x.ToFloat();
        const float tCenterY = (tTrans.position + tBox.offset).y.ToFloat();
        const float tCenterZ = (tTrans.position + tBox.offset).z.ToFloat();
        const float aCenterX = (aTrans.position + aBox.offset).x.ToFloat();
        const float aCenterY = (aTrans.position + aBox.offset).y.ToFloat();
        const float aCenterZ = (aTrans.position + aBox.offset).z.ToFloat();

        return std::abs(tCenterX - aCenterX) < (tHalfX + aHalfX) && std::abs(tCenterY - aCenterY) < (tHalfY + aHalfY) &&
               std::abs(tCenterZ - aCenterZ) < (tHalfZ + aHalfZ);
    }

    std::vector<PendingEvent> m_pendingEvents;
};

} // namespace urpg
