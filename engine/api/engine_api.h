#pragma once

#include "engine/core/render/i_renderer.h"
#include <map>
#include <string>

namespace urpg {

/**
 * @brief High-level API for games built on this engine.
 * Simplifies setup and provides clean access to core features.
 */
class EngineAPI {
public:
    static EngineAPI& getInstance() {
        static EngineAPI instance;
        return instance;
    }

    // Entity creation helpers
    EntityID createPlayer(World& world, float x, float y) {
        EntityID id = world.CreateEntity();
        world.AddComponent<TransformComponent>(id, { Vector3::FromFloats(x, y, 0) });
        world.AddComponent<VelocityComponent>(id, { Vector3(0, 0, 0) });
        world.AddComponent<PlayerControlComponent>(id, { 300.0f });
        world.AddComponent<SpriteComponent>(id, { "player", 0, 0, 32, 32 });
        world.AddComponent<InventoryComponent>(id, {});
        world.AddComponent<StatsComponent>(id, { 100, 100, 1, 0, 100 });
        world.AddComponent<CameraFollowComponent>(id, { 1.0f });
        return id;
    }

    EntityID createMerchant(World& world, float x, float y, const std::string& name) {
        EntityID id = world.CreateEntity();
        world.AddComponent<TransformComponent>(id, { Vector3::FromFloats(x, y, 0) });
        world.AddComponent<SpriteComponent>(id, { "merchant", 0, 0, 32, 32 });
        world.AddComponent<MerchantComponent>(id, { 1.0f, 0.8f });
        world.AddComponent<InteractionComponent>(id, { 50.0f, "Talk" });
        return id;
    }

    // Asset Management
    void setWindowTitle(const std::string& title) {
        m_title = title;
    }

private:
    EngineAPI() = default;
    std::string m_title = "URPG Game";
};

} // namespace urpg
