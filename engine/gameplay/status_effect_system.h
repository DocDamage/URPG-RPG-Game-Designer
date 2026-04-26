#pragma once

#include "engine/core/ecs/health_components.h"
#include "engine/core/ecs/world.h"
#include "engine/gameplay/status_effects.h"

namespace urpg {

/**
 * @brief System that processes active status effects on entities.
 */
class StatusEffectSystem {
  public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));

        world.ForEachWith<HealthComponent, StatusEffectComponent>(
            [&](EntityID, HealthComponent& health, StatusEffectComponent& effect) {
                if (effect.duration <= Fixed32::FromInt(0)) {
                    // Effect expired - logic to remove component should be here
                    return;
                }

                effect.duration = effect.duration - dt;
                effect.lastTickTime = effect.lastTickTime + dt;

                if (effect.lastTickTime >= effect.tickInterval) {
                    effect.lastTickTime = Fixed32::FromInt(0);

                    switch (effect.type) {
                    case StatusEffectComponent::EffectType::Poison:
                    case StatusEffectComponent::EffectType::Burn:
                        health.current -= effect.power;
                        break;
                    case StatusEffectComponent::EffectType::Regen:
                        health.current += effect.power;
                        if (health.current > health.max)
                            health.current = health.max;
                        break;
                    case StatusEffectComponent::EffectType::Stun:
                        // Stun logic would typically disable AISystem or PlayerControlSystem for this entity
                        break;
                    }

                    if (health.current <= 0) {
                        health.current = 0;
                        health.isAlive = false;
                    }
                }
            });
    }
};

} // namespace urpg
