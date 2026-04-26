#include <catch2/catch_test_macros.hpp>

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/editor/plugin_api.h"
#include "engine/core/editor/security_manager.h"
#include "engine/core/global_state_hub.h"
#include "runtimes/compat_js/input_manager.h"

namespace {

bool worldContainsEntity(urpg::World& world, urpg::EntityID target) {
    bool found = false;
    world.ForEachWith<>([&](urpg::EntityID entityId) {
        if (entityId == target) {
            found = true;
        }
    });
    return found;
}

std::size_t worldEntityCount(urpg::World& world) {
    std::size_t count = 0;
    world.ForEachWith<>([&](urpg::EntityID) { ++count; });
    return count;
}

} // namespace

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
    world.ForEachWith<>([&](urpg::EntityID) { ++alive_count; });
    REQUIRE(alive_count == 1);

    urpg::editor::UnbindPluginAPIWorld();
    REQUIRE(URPG_EntityCreate() == 0);
}

TEST_CASE("Plugin API scoped world bindings restore the previous world", "[plugin_api]") {
    urpg::World outerWorld;
    urpg::World innerWorld;

    {
        urpg::editor::ScopedPluginAPIWorldBinding outerBinding(&outerWorld);
        const auto outerId = URPG_EntityCreate();
        REQUIRE(outerId != 0);
        REQUIRE(worldContainsEntity(outerWorld, static_cast<urpg::EntityID>(outerId)));

        {
            urpg::editor::ScopedPluginAPIWorldBinding innerBinding(&innerWorld);
            const auto innerId = URPG_EntityCreate();
            REQUIRE(innerId != 0);
            REQUIRE(worldContainsEntity(innerWorld, static_cast<urpg::EntityID>(innerId)));
            REQUIRE(worldEntityCount(outerWorld) == 1);
            REQUIRE(worldEntityCount(innerWorld) == 1);
        }

        const auto restoredOuterId = URPG_EntityCreate();
        REQUIRE(restoredOuterId != 0);
        REQUIRE(worldContainsEntity(outerWorld, static_cast<urpg::EntityID>(restoredOuterId)));
        REQUIRE(worldEntityCount(outerWorld) == 2);
        REQUIRE(worldEntityCount(innerWorld) == 1);
    }

    REQUIRE(URPG_EntityCreate() == 0);
}

TEST_CASE("Plugin API ignores null world binds and unbalanced unbinds", "[plugin_api]") {
    urpg::editor::UnbindPluginAPIWorld();
    urpg::editor::BindPluginAPIWorld(nullptr);
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

TEST_CASE("Plugin API logging emits runtime diagnostics", "[plugin_api][diagnostics]") {
    urpg::diagnostics::RuntimeDiagnostics::clear();

    URPG_LogInfo("plugin ready");
    URPG_LogError("plugin failed");

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(diagnostics.size() == 2);

    REQUIRE(diagnostics[0].severity == urpg::diagnostics::DiagnosticSeverity::Info);
    REQUIRE(diagnostics[0].subsystem == "editor.plugin_api");
    REQUIRE(diagnostics[0].code == "plugin.log.info");
    REQUIRE(diagnostics[0].message == "plugin ready");

    REQUIRE(diagnostics[1].severity == urpg::diagnostics::DiagnosticSeverity::Error);
    REQUIRE(diagnostics[1].subsystem == "editor.plugin_api");
    REQUIRE(diagnostics[1].code == "plugin.log.error");
    REQUIRE(diagnostics[1].message == "plugin failed");

    urpg::diagnostics::RuntimeDiagnostics::clear();
}

TEST_CASE("Plugin security audit emits runtime diagnostics", "[plugin_api][security][diagnostics]") {
    auto& security = urpg::editor::PluginSecurityManager::instance();
    security.setDefaultPolicy({});
    security.revokeAll("diagnostic_security_fixture");
    urpg::diagnostics::RuntimeDiagnostics::clear();

    REQUIRE_FALSE(
        security.requestPermission("diagnostic_security_fixture", urpg::editor::PluginPermission::NetworkAccess));

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics[0].severity == urpg::diagnostics::DiagnosticSeverity::Warning);
    REQUIRE(diagnostics[0].subsystem == "editor.security");
    REQUIRE(diagnostics[0].code == "plugin.permission.denied");
    REQUIRE(diagnostics[0].message.find("diagnostic_security_fixture") != std::string::npos);
    REQUIRE(diagnostics[0].message.find("NetworkAccess") != std::string::npos);

    urpg::diagnostics::RuntimeDiagnostics::clear();
    security.revokeAll("diagnostic_security_fixture");
}
