#include <catch2/catch_test_macros.hpp>

#include "engine/core/mod/mod_registry.h"

using namespace urpg::mod;

TEST_CASE("ModRegistry: Register and retrieve mod", "[mod]") {
    ModRegistry registry;

    ModManifest manifest;
    manifest.id = "test_mod";
    manifest.name = "Test Mod";
    manifest.version = "1.0.0";

    REQUIRE(registry.registerMod(manifest));

    auto retrieved = registry.getMod("test_mod");
    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved->id == "test_mod");
    REQUIRE(retrieved->name == "Test Mod");
    REQUIRE(retrieved->version == "1.0.0");
}

TEST_CASE("ModRegistry: Duplicate registration returns false", "[mod]") {
    ModRegistry registry;

    ModManifest manifest;
    manifest.id = "dup_mod";
    manifest.name = "Duplicate Mod";
    manifest.version = "1.0.0";

    REQUIRE(registry.registerMod(manifest));
    REQUIRE_FALSE(registry.registerMod(manifest));
}

TEST_CASE("ModRegistry: Dependency resolution produces correct load order", "[mod]") {
    ModRegistry registry;

    ModManifest core;
    core.id = "core";
    core.name = "Core";
    core.version = "1.0.0";

    ModManifest addon;
    addon.id = "addon";
    addon.name = "Addon";
    addon.version = "1.0.0";
    addon.dependencies = {"core"};

    ModManifest plugin;
    plugin.id = "plugin";
    plugin.name = "Plugin";
    plugin.version = "1.0.0";
    plugin.dependencies = {"core"};

    REQUIRE(registry.registerMod(core));
    REQUIRE(registry.registerMod(addon));
    REQUIRE(registry.registerMod(plugin));

    auto order = registry.resolveLoadOrder();
    REQUIRE(order.size() == 3);
    REQUIRE(order[0] == "core");
}

TEST_CASE("ModRegistry: Circular dependency throws on resolve", "[mod]") {
    ModRegistry registry;

    ModManifest a;
    a.id = "mod_a";
    a.name = "Mod A";
    a.version = "1.0.0";
    a.dependencies = {"mod_b"};

    ModManifest b;
    b.id = "mod_b";
    b.name = "Mod B";
    b.version = "1.0.0";
    b.dependencies = {"mod_a"};

    REQUIRE(registry.registerMod(a));
    REQUIRE(registry.registerMod(b));

    REQUIRE_THROWS_AS(registry.resolveLoadOrder(), std::runtime_error);
}

TEST_CASE("ModRegistry: Activate and deactivate state tracking", "[mod]") {
    ModRegistry registry;

    ModManifest manifest;
    manifest.id = "toggle_mod";
    manifest.name = "Toggle Mod";
    manifest.version = "1.0.0";

    REQUIRE(registry.registerMod(manifest));

    REQUIRE(registry.listActiveMods().empty());

    REQUIRE(registry.activateMod("toggle_mod"));
    auto active = registry.listActiveMods();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == "toggle_mod");

    REQUIRE(registry.deactivateMod("toggle_mod"));
    REQUIRE(registry.listActiveMods().empty());

    REQUIRE_FALSE(registry.activateMod("nonexistent"));
    REQUIRE_FALSE(registry.deactivateMod("nonexistent"));
}

TEST_CASE("ModRegistry: Save and load state round-trip", "[mod]") {
    ModRegistry registry;

    ModManifest manifest;
    manifest.id = "persist_mod";
    manifest.name = "Persist Mod";
    manifest.version = "1.0.0";

    REQUIRE(registry.registerMod(manifest));
    REQUIRE(registry.activateMod("persist_mod"));

    auto state = registry.saveState();
    REQUIRE(state["version"] == "1.0.0");
    REQUIRE(state["mods"].is_array());
    REQUIRE(state["mods"].size() == 1);
    REQUIRE(state["mods"][0]["id"] == "persist_mod");
    REQUIRE(state["mods"][0]["active"] == true);

    ModRegistry loadedRegistry;
    loadedRegistry.loadState(state);

    auto retrieved = loadedRegistry.getMod("persist_mod");
    REQUIRE(retrieved.has_value());

    auto active = loadedRegistry.listActiveMods();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == "persist_mod");
}

TEST_CASE("ModRegistry: Unregister removes mod and breaks dependency chains gracefully", "[mod]") {
    ModRegistry registry;

    ModManifest core;
    core.id = "core";
    core.name = "Core";
    core.version = "1.0.0";

    ModManifest dependent;
    dependent.id = "dependent";
    dependent.name = "Dependent";
    dependent.version = "1.0.0";
    dependent.dependencies = {"core"};

    REQUIRE(registry.registerMod(core));
    REQUIRE(registry.registerMod(dependent));

    REQUIRE(registry.unregisterMod("core"));

    REQUIRE_FALSE(registry.getMod("core").has_value());
    REQUIRE(registry.getMod("dependent").has_value());

    auto order = registry.resolveLoadOrder();
    REQUIRE(order.size() == 1);
    REQUIRE(order[0] == "dependent");
}
