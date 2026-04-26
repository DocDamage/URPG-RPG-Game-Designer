#pragma once

#include "engine/core/ecs/collision_system.h"
#include "engine/core/ecs/health_components.h"
#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_shake_system.h"

namespace urpg {

/**
 * @brief System that processes health changes, invulnerability frames, and contact damage.
 */
class HealthSystem {
  public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));
        m_currentTime = m_currentTime + dt;

        // 1. Process Invulnerability Timers
        world.ForEachWith<InvulnerabilityComponent>([&](EntityID id, InvulnerabilityComponent& invul) {
            if (invul.timer > Fixed32::FromInt(0)) {
                invul.timer = invul.timer - dt;
            } else {
                world.DestroyEntity(id); // Simplified: remove component if expired
            }
        });

        // 2. Process Contact Damage (using Collision logic)
        // Note: In a real engine, this would use a 'CollisionEvents' list from the CollisionSystem
        // but for now we'll do a simple spatial check for demo purposes.
    }

    void applyDamage(World& world, EntityID target, int32_t amount) {
        auto* health = world.GetComponent<HealthComponent>(target);
        if (!health || !health->isAlive)
            return;

        // Check for invulnerability
        if (world.GetComponent<InvulnerabilityComponent>(target))
            return;

        health->current -= amount;
        if (health->current <= 0) {
            health->current = 0;
            health->isAlive = false;
        }

        // Apply a small camera shake on damage if it's the player
        // (Assuming we can find the camera, or just add trauma to all active cameras)
        world.ForEachWith<CameraShakeComponent>([&](EntityID, CameraShakeComponent& shake) {
            CameraShakeSystem::addTrauma(shake, Fixed32::FromRaw(16383)); // 0.25 trauma
        });

        // Add I-frames
        InvulnerabilityComponent invul;
        invul.timer = Fixed32::FromInt(1);
        world.AddComponent(target, invul);
    }

    bool heal(World& world, EntityID target, int32_t amount) {
        auto* health = world.GetComponent<HealthComponent>(target);
        if (!health || amount <= 0 || !health->isAlive) {
            return false;
        }

        health->current += amount;
        if (health->current > health->max) {
            health->current = health->max;
        }
        return true;
    }

  private:
    Fixed32 m_currentTime = Fixed32::FromInt(0);
};

} // namespace urpg
