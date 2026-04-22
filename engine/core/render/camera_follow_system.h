#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/render/camera_components.h"
#include "engine/core/render/camera_follow_components.h"

namespace urpg {

/**
 * @brief System that updates camera positions based on their follow target.
 */
class CameraFollowSystem {
public:
    void update(World& world, [[maybe_unused]] float deltaTime) {
        world.ForEachWith<TransformComponent, CameraComponent, CameraFollowComponent>([&](EntityID, TransformComponent& cameraTransform, const CameraComponent& camera, const CameraFollowComponent& follow) {
            if (!camera.isActive || follow.target == 0) return;

            auto* targetTransform = world.GetComponent<TransformComponent>(follow.target);
            if (!targetTransform) return;

            Vector3 desiredPosition = targetTransform->position + follow.offset;

            switch (follow.mode) {
                case CameraFollowMode::Static:
                    // Do nothing, position stays fixed
                    break;
                case CameraFollowMode::Simple:
                    cameraTransform.position = desiredPosition;
                    break;
                case CameraFollowMode::Smooth:
                    // Very basic lerp: current + (desired - current) * smoothFactor
                    cameraTransform.position = cameraTransform.position + (desiredPosition - cameraTransform.position) * follow.smoothFactor;
                    break;
                case CameraFollowMode::Delayed:
                    // For now, same as smooth until we add a deadzone
                    cameraTransform.position = cameraTransform.position + (desiredPosition - cameraTransform.position) * follow.smoothFactor;
                    break;
            }
        });
    }
};

} // namespace urpg
