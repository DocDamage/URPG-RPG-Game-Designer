#include <catch2/catch_test_macros.hpp>

#include "editor/mod/mod_manager_panel.h"
#include "engine/core/mod/mod_registry.h"

using namespace urpg::editor;
using namespace urpg::mod;

TEST_CASE("ModManagerPanel: Empty snapshot when no registry bound", "[mod][editor][panel]") {
    ModManagerPanel panel;
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["registered_count"] == 0);
    REQUIRE(snapshot["active_count"] == 0);
    REQUIRE(snapshot["resolved_load_order"].empty());
    REQUIRE(snapshot["cycle_warning"].is_null());
}

TEST_CASE("ModManagerPanel: Snapshot reflects mods after bind", "[mod][editor][panel]") {
    ModRegistry registry;

    ModManifest manifest;
    manifest.id = "ui_mod";
    manifest.name = "UI Mod";
    manifest.version = "1.0.0";

    REQUIRE(registry.registerMod(manifest));
    REQUIRE(registry.activateMod("ui_mod"));

    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["registered_count"] == 1);
    REQUIRE(snapshot["active_count"] == 1);
    REQUIRE(snapshot["registered_mod_ids"].size() == 1);
    REQUIRE(snapshot["registered_mod_ids"][0] == "ui_mod");
    REQUIRE(snapshot["validation_issue_count"] == 1);
    REQUIRE(snapshot["validation_issues"].size() == 1);
    REQUIRE(snapshot["validation_issues"][0]["category"] == "missing_entry_point");
    REQUIRE(snapshot["resolved_load_order"].size() == 1);
    REQUIRE(snapshot["resolved_load_order"][0] == "ui_mod");
    REQUIRE(snapshot["cycle_warning"].is_null());
}

TEST_CASE("ModManagerPanel: Cycle warning appears in snapshot when cyclic mods are registered", "[mod][editor][panel]") {
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

    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot["cycle_warning"].is_null());
    REQUIRE(snapshot["cycle_warning"].get<std::string>().find("circular dependency") != std::string::npos);
    REQUIRE(snapshot["registered_count"] == 2);
}

TEST_CASE("ModManagerPanel: Validation issues are surfaced in snapshot", "[mod][editor][panel]") {
    ModRegistry registry;

    ModManifest invalid;
    invalid.id = "bad_mod";
    invalid.dependencies = {"bad_mod", "extra_dep", "extra_dep"};

    REQUIRE(registry.registerMod(invalid));

    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["validation_issue_count"] == 5);
    REQUIRE(snapshot["validation_issues"].size() == 5);
    REQUIRE(snapshot["validation_issues"][0]["category"] == "empty_name");
    REQUIRE(snapshot["validation_issues"][1]["category"] == "empty_version");
    REQUIRE(snapshot["validation_issues"][2]["category"] == "missing_entry_point");
    REQUIRE(snapshot["validation_issues"][3]["category"] == "self_dependency");
    REQUIRE(snapshot["validation_issues"][4]["category"] == "duplicate_dependency");
}
