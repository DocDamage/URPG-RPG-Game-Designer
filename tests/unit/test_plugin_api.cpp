#include <catch2/catch_test_macros.hpp>

#include "engine/core/editor/plugin_api.h"
#include "engine/core/global_state_hub.h"

TEST_CASE("Plugin API uses scratch-state storage rather than GlobalStateHub", "[plugin_api]") {
    auto& hub = urpg::GlobalStateHub::getInstance();
    hub.resetAll();

    hub.setVariable("coins", 77);
    hub.setSwitch("door_open", true);

    REQUIRE(URPG_GetGlobalVariable("coins") == 0.0f);
    REQUIRE_FALSE(URPG_GetGlobalSwitch("door_open"));

    URPG_SetGlobalVariable("coins", 12.5f);
    URPG_SetGlobalSwitch("door_open", true);

    REQUIRE(URPG_GetGlobalVariable("coins") == 12.5f);
    REQUIRE(URPG_GetGlobalSwitch("door_open"));

    auto hubCoins = hub.getVariable("coins");
    REQUIRE(std::get<int32_t>(hubCoins) == 77);
    REQUIRE(hub.getSwitch("door_open"));
}

TEST_CASE("Plugin API entity and input exports remain placeholder-backed", "[plugin_api]") {
    const auto firstId = URPG_EntityCreate();
    const auto secondId = URPG_EntityCreate();

    REQUIRE(secondId == firstId + 1);
    REQUIRE_FALSE(URPG_IsKeyPressed(13));

    float x = -1.0f;
    float y = -1.0f;
    URPG_GetMousePosition(&x, &y);
    REQUIRE(x == 0.0f);
    REQUIRE(y == 0.0f);

    URPG_EntityDestroy(firstId);
    URPG_EntityAddComponent(secondId, "Transform");
}
