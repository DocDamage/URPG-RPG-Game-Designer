#include <catch2/catch_test_macros.hpp>

#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/editor/plugin_api.h"
#include "engine/core/global_state_hub.h"
#include "runtimes/compat_js/input_manager.h"

TEST_CASE("Plugin API global-state exports route through GlobalStateHub", "[plugin_api]") {
    auto& hub = urpg::GlobalStateHub::getInstance();
    hub.resetAll();

    hub.setVariable("coins", 77);
    hub.setSwitch("door_open", true);

    REQUIRE(URPG_GetGlobalVariable("coins") == 77.0f);
    REQUIRE(URPG_GetGlobalSwitch("door_open"));

    URPG_SetGlobalVariable("coins", 12.5f);
    URPG_SetGlobalSwitch("door_open", false);

    REQUIRE(URPG_GetGlobalVariable("coins") == 12.5f);
    REQUIRE_FALSE(URPG_GetGlobalSwitch("door_open"));

    auto hubCoins = hub.getVariable("coins");
    REQUIRE(std::get<float>(hubCoins) == 12.5f);
    REQUIRE_FALSE(hub.getSwitch("door_open"));
}

TEST_CASE("Plugin API entity exports operate on a bound ECS world", "[plugin_api]") {
    urpg::World world;
    urpg::editor::BindPluginAPIWorld(&world);

    const auto firstId = URPG_EntityCreate();
    const auto secondId = URPG_EntityCreate();

    REQUIRE(firstId != 0);
    REQUIRE(secondId == firstId + 1);

    URPG_EntityAddComponent(firstId, "Transform");
    URPG_EntityAddComponent(firstId, "Velocity");
    URPG_EntityAddComponent(firstId, "PlayerControl");

    REQUIRE(world.HasComponent<urpg::TransformComponent>(static_cast<urpg::EntityID>(firstId)));
    REQUIRE(world.HasComponent<urpg::VelocityComponent>(static_cast<urpg::EntityID>(firstId)));
    REQUIRE(world.HasComponent<urpg::PlayerControlComponent>(static_cast<urpg::EntityID>(firstId)));

    URPG_EntityDestroy(firstId);

    uint32_t alive_count = 0;
    world.ForEachWith<>([&](urpg::EntityID) {
        ++alive_count;
    });
    REQUIRE(alive_count == 1);

    urpg::editor::UnbindPluginAPIWorld();
    REQUIRE(URPG_EntityCreate() == 0);
}

TEST_CASE("Plugin API input exports read compat InputManager state", "[plugin_api]") {
    auto& input = urpg::compat::InputManager::instance();
    input.initialize();
    input.clear();

    input.setKeyPressed(13, true);
    input.setMousePosition(144, 288);

    REQUIRE(URPG_IsKeyPressed(13));
    REQUIRE_FALSE(URPG_IsKeyPressed(27));

    float x = -1.0f;
    float y = -1.0f;
    URPG_GetMousePosition(&x, &y);
    REQUIRE(x == 144.0f);
    REQUIRE(y == 288.0f);

    input.shutdown();
}
