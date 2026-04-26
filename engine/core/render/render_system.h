#pragma once

#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_system.h"
#include "engine/core/render/i_renderer.h"
#include "engine/core/render/render_components.h"

namespace urpg {

/**
 * @brief Bridges the ECS world state to the platform-specific Renderer.
 * This is the "Backend" system that actually issues draw calls.
 */
class RenderSystem {
  public:
    void render(World& world, IRenderer& renderer, const CameraSystem& camera) {
        // Clear and Present moved to EngineHost for scene/entity layering

        // Set camera view matrix/offset
        renderer.setCamera(camera.getPosition().x.ToFloat(), camera.getPosition().y.ToFloat(),
                           1.0f); // Zoom could be added to CameraComponent

        // Draw all entities with a SpriteComponent and TransformComponent
        world.ForEachWith<TransformComponent, SpriteComponent>(
            [&](EntityID id, TransformComponent& trans, SpriteComponent& sprite) {
                if (!sprite.visible)
                    return;

                // Simple culling could be added here

                renderer.drawSprite(sprite.textureId, sprite.srcX, sprite.srcY, sprite.width, sprite.height,
                                    trans.position.x.ToFloat(), trans.position.y.ToFloat(),
                                    static_cast<float>(sprite.width), static_cast<float>(sprite.height), sprite.flipX);
            });
    }
};

} // namespace urpg
