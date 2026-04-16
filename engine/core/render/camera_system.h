#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/render/camera_components.h"

namespace urpg {

/**
 * @brief System that tracks the active camera and prepares view matrices.
 */
class CameraSystem {
public:
    void update(World& world) {
        m_activeCameraFound = false;
        
        world.ForEachWith<TransformComponent, CameraComponent>([&](EntityID id, const TransformComponent& transform, const CameraComponent& camera) {
            if (camera.isActive && !m_activeCameraFound) {
                m_viewPosition = transform.position;
                m_viewRotation = transform.rotation;
                m_activeCameraData = camera;
                m_activeCameraFound = true;
                m_activeEntity = id;
            }
        });
    }

    bool hasActiveCamera() const { return m_activeCameraFound; }
    Vector3 getViewPosition() const { return m_viewPosition; }
    Vector3 getViewRotation() const { return m_viewRotation; }
    CameraComponent getCameraData() const { return m_activeCameraData; }
    EntityID getActiveEntity() const { return m_activeEntity; }

private:
    bool m_activeCameraFound = false;
    Vector3 m_viewPosition;
    Vector3 m_viewRotation;
    CameraComponent m_activeCameraData;
    EntityID m_activeEntity;
};

} // namespace urpg
