#pragma once

#include "engine/core/ecs/world.h"

namespace urpg {

/**
 * @brief Component used to track an entity's "LifeTime" before automatic destruction.
 * Useful for time-limited projectiles or transient effects.
 */
struct LifeTimeComponent {
    Fixed32 duration;
};

class LifeTimeSystem {
public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));
        std::vector<EntityID> toDestroy;

        world.ForEachWith<LifeTimeComponent>([&](EntityID id, LifeTimeComponent& life) {
            life.duration = life.duration - dt;
            if (life.duration <= Fixed32::FromInt(0)) {
                toDestroy.push_back(id);
            }
        });

        for (auto id : toDestroy) {
            world.DestroyEntity(id);
        }
    }
};

} // namespace urpg
