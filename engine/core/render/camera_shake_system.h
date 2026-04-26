#pragma once

#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_shake_components.h"
#include <cmath>

namespace urpg {

/**
 * @brief System that applies procedural shake effects to cameras based on trauma level.
 * Uses a common trauma-based approach: shake = trauma^2 * noise.
 */
class CameraShakeSystem {
  public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));
        m_timer += deltaTime;

        world.ForEachWith<TransformComponent, CameraShakeComponent>(
            [&](EntityID, TransformComponent& transform, CameraShakeComponent& shake) {
                if (shake.trauma <= Fixed32::FromInt(0))
                    return;

                // 1. Decay trauma over time
                Fixed32 decayAmount = shake.traumaDecay * dt;
                if (shake.trauma > decayAmount) {
                    shake.trauma = shake.trauma - decayAmount;
                } else {
                    shake.trauma = Fixed32::FromInt(0);
                }

                // 2. Calculate shake intensity (trauma^2)
                float t = shake.trauma.ToFloat();
                float shakeIntensity = t * t;

                // 3. Apply displacement based on simple deterministic noise (sine waves for demo)
                // In a full implementation, this should use a Perlin noise table for better feel
                if (shake.mode == CameraShakeMode::Positional || shake.mode == CameraShakeMode::Both) {
                    float offsetX = std::sin(m_timer * 25.0f) * shakeIntensity * shake.maxTranslation.ToFloat();
                    float offsetY = std::cos(m_timer * 27.0f) * shakeIntensity * shake.maxTranslation.ToFloat();

                    transform.position.x =
                        transform.position.x + Fixed32::FromRaw(static_cast<int32_t>(offsetX * 65536.0f));
                    transform.position.y =
                        transform.position.y + Fixed32::FromRaw(static_cast<int32_t>(offsetY * 65536.0f));
                }

                if (shake.mode == CameraShakeMode::Rotational || shake.mode == CameraShakeMode::Both) {
                    float offsetR = std::sin(m_timer * 30.0f) * shakeIntensity * shake.maxRotation.ToFloat();
                    transform.rotation.z =
                        transform.rotation.z + Fixed32::FromRaw(static_cast<int32_t>(offsetR * 65536.0f));
                }
            });
    }

    /**
     * @brief Utility helper to add trauma to a camera.
     */
    static void addTrauma(CameraShakeComponent& shake, Fixed32 amount) {
        shake.trauma = shake.trauma + amount;
        if (shake.trauma > Fixed32::FromInt(1)) {
            shake.trauma = Fixed32::FromInt(1);
        }
    }

  private:
    float m_timer = 0.0f;
};

} // namespace urpg
