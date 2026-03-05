#include "engine/core/threading/thread_roles.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Thread roles map to correct script access", "[threading]") {
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Logic) == urpg::ScriptAccess::Full);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::AssetStreaming) == urpg::ScriptAccess::CallbackOnly);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Render) == urpg::ScriptAccess::None);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Audio) == urpg::ScriptAccess::None);
}

TEST_CASE("Thread registry identifies current role", "[threading]") {
    urpg::ThreadRegistry registry;
    registry.RegisterCurrentThread(urpg::ThreadRole::Logic);

    REQUIRE(registry.IsCurrentThread(urpg::ThreadRole::Logic));
    REQUIRE(registry.CurrentScriptAccess() == urpg::ScriptAccess::Full);
    REQUIRE(registry.CanRunScriptDirectly());
}
