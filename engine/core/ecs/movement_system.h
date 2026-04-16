#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"

namespace urpg {

class MovementSystem {
public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));
        
        world.ForEachWith<TransformComponent, VelocityComponent>([&](EntityID, TransformComponent& transform, VelocityComponent& velocity) {
            transform.position = transform.position + (velocity.linear * dt);
            transform.rotation = transform.rotation + (velocity.angular * dt);
        });
    }
};

} // namespace urpg
