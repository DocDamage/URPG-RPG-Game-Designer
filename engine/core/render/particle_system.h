#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/render/particle_components.h"
#include "engine/core/ecs/actor_components.h"
#include <vector>

namespace urpg {

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Fixed32 remainingLifetime;
};

/**
 * @brief System that simulates particle effects attached to entities.
 */
class ParticleSystem {
public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));

        world.ForEachWith<TransformComponent, ParticleEmitterComponent>([&](EntityID id, const TransformComponent& trans, const ParticleEmitterComponent& emitter) {
            // Particle simulation logic would happen here.
            // For now, we gather active emitters for the renderer.
        });
    }
};

} // namespace urpg
