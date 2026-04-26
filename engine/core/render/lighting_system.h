#pragma once

#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/world.h"
#include "engine/core/render/lighting_components.h"
#include <vector>

namespace urpg {

struct RenderableLight {
    Vector3 position;
    Vector3 color;
    Fixed32 intensity;
    Fixed32 radius;
};

/**
 * @brief System responsible for gathering and processing light data for the renderer.
 */
class LightingSystem {
  public:
    void update(World& world) {
        m_visibleLights.clear();

        // Find global ambient light (assume one entity has it)
        world.ForEachWith<AmbientLightComponent>([&](EntityID, const AmbientLightComponent& ambient) {
            m_ambientColor = ambient.color;
            m_ambientIntensity = ambient.intensity;
        });

        // Gather all point lights
        world.ForEachWith<TransformComponent, PointLightComponent>(
            [&](EntityID, const TransformComponent& transform, const PointLightComponent& light) {
                m_visibleLights.push_back({transform.position, light.color, light.intensity, light.radius});
            });
    }

    const std::vector<RenderableLight>& getVisibleLights() const { return m_visibleLights; }
    Vector3 getAmbientColor() const { return m_ambientColor; }
    Fixed32 getAmbientIntensity() const { return m_ambientIntensity; }

  private:
    std::vector<RenderableLight> m_visibleLights;
    Vector3 m_ambientColor;
    Fixed32 m_ambientIntensity;
};

} // namespace urpg
