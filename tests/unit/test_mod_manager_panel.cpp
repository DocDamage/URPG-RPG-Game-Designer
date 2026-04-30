#include <catch2/catch_test_macros.hpp>

#include "editor/mod/mod_manager_panel.h"
#include "engine/core/mod/mod_hot_loader.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/mod/mod_store.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

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
    REQUIRE(snapshot["registry_bound"] == false);
    REQUIRE(snapshot["status"] == "disabled");
    REQUIRE(snapshot["disabled_reason"] == "No mod registry is bound.");
    REQUIRE(snapshot["loader_bound"] == false);
    REQUIRE(snapshot["actions"]["import_manifest"] == false);
    REQUIRE(snapshot["actions"]["poll_hot_loader"] == false);
    REQUIRE(snapshot["status_messages"].size() == 4);
    REQUIRE(snapshot["status_messages"][0] == "No mod registry is bound.");
    REQUIRE(snapshot["status_messages"][1] == "No mod loader is bound; mod actions are disabled.");
    REQUIRE(snapshot["status_messages"][2] == "No mod hot-loader is bound; live reload event log is disabled.");
    REQUIRE(snapshot["status_messages"][3] == "No mod store catalog is bound; store install actions are disabled.");
    REQUIRE(snapshot["hot_loader_bound"] == false);
    REQUIRE(snapshot["hot_load"]["event_count"] == 0);
}

TEST_CASE("ModManagerPanel: Empty registry snapshot exposes empty state", "[mod][editor][panel][empty]") {
    ModRegistry registry;
    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["status"] == "empty");
    REQUIRE(snapshot["empty_reason"] == "No mods are registered.");
    REQUIRE(snapshot["error_message"] == "");
    REQUIRE(snapshot["registered_count"] == 0);
    REQUIRE(snapshot["mods"].empty());
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
    REQUIRE(snapshot["status"] == "error");
    REQUIRE_FALSE(snapshot["error_message"].get<std::string>().empty());
    REQUIRE(snapshot["validation_issues"].size() == 1);
    REQUIRE(snapshot["validation_issues"][0]["category"] == "missing_entry_point");
    REQUIRE(snapshot["resolved_load_order"].size() == 1);
    REQUIRE(snapshot["resolved_load_order"][0] == "ui_mod");
    REQUIRE(snapshot["cycle_warning"].is_null());
}

TEST_CASE("ModManagerPanel: Import, activate, deactivate, and reload use the bound loader",
          "[mod][editor][panel][actions]") {
    ModRegistry registry;
    ModLoader loader(registry);
    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.bindLoader(&loader);

    ModSandboxPolicy policy;
    policy.allowFileSystemRead = true;

    const auto manifestPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / "mod_manifest_fixture.json";

    REQUIRE(panel.importManifest(manifestPath, policy));

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["loader_bound"] == true);
    REQUIRE(snapshot["registered_count"] == 1);
    REQUIRE(snapshot["active_count"] == 0);
    REQUIRE(snapshot["last_action"]["action"] == "register_manifest");
    REQUIRE(snapshot["last_action"]["success"] == true);
    REQUIRE(snapshot["mods"].size() == 1);
    REQUIRE(snapshot["mods"][0]["id"] == "core_ui");
    REQUIRE(snapshot["mods"][0]["entry_point"] == "mods/core_ui/main.js");
    REQUIRE(snapshot["mods"][0]["sandbox_policy"]["file_system_read"] == "allowed");
    REQUIRE(snapshot["mods"][0]["sandbox_policy"]["network_access"] == "denied");
    REQUIRE(snapshot["mods"][0]["contract_failures"].empty());

    REQUIRE(panel.activateMod("core_ui"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["active_count"] == 1);
    REQUIRE(snapshot["mods"][0]["active"] == true);
    REQUIRE(snapshot["last_action"]["action"] == "activate");
    REQUIRE(snapshot["last_action"]["load_order"].size() == 1);

    REQUIRE(panel.deactivateMod("core_ui"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["active_count"] == 0);
    REQUIRE(snapshot["mods"][0]["active"] == false);
    REQUIRE(snapshot["last_action"]["action"] == "deactivate");

    REQUIRE(panel.reloadMod("core_ui"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["registered_count"] == 1);
    REQUIRE(snapshot["active_count"] == 1);
    REQUIRE(snapshot["last_action"]["action"] == "reload");
    REQUIRE(snapshot["mods"][0]["sandbox_policy"]["file_system_read"] == "allowed");
}

TEST_CASE("ModManagerPanel: Loader errors and contract failures are surfaced", "[mod][editor][panel][actions]") {
    ModRegistry registry;
    ModLoader loader(registry);
    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.bindLoader(&loader);

    ModManifest invalid;
    invalid.id = "bad_mod";
    invalid.name = "Bad Mod";
    invalid.version = "1.0.0";

    REQUIRE_FALSE(panel.registerManifest(invalid));
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["last_action"]["success"] == false);
    REQUIRE(snapshot["last_action"]["error"].get<std::string>().find("Contract violation") != std::string::npos);
    REQUIRE(snapshot["registered_count"] == 0);

    REQUIRE_FALSE(panel.activateMod("missing_mod"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["last_action"]["action"] == "activate");
    REQUIRE(snapshot["last_action"]["error"].get<std::string>().find("Unknown mod") != std::string::npos);

    REQUIRE_FALSE(panel.importManifest("content/fixtures/missing_manifest.json"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["last_action"]["action"] == "import_manifest");
    REQUIRE(snapshot["last_action"]["error"].get<std::string>().find("Failed to open manifest") != std::string::npos);
}

TEST_CASE("ModManagerPanel: Cycle warning appears in snapshot when cyclic mods are registered",
          "[mod][editor][panel]") {
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

TEST_CASE("ModManagerPanel: Hot-load event log is surfaced and can reload active mods",
          "[mod][editor][panel][hotload]") {
    const auto tempDir =
        std::filesystem::temp_directory_path() /
        ("urpg_mod_manager_hotload_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(tempDir);
    const auto scriptPath = tempDir / "editor_hot_mod.js";
    {
        std::ofstream script(scriptPath, std::ios::binary);
        REQUIRE(script.is_open());
        script << "export function activate(context) { context.commands.register('editor.hot.echo', function(v) { return 'v1:' + v; }); }";
    }

    ModManifest manifest;
    manifest.id = "urpg.editor.hot";
    manifest.name = "Editor Hot Mod";
    manifest.version = "1.0.0";
    manifest.entryPoint = scriptPath.string();

    ModRegistry registry;
    ModLoader loader(registry);
    ModHotLoader hotLoader(loader, registry);
    hotLoader.setDebounceInterval(std::chrono::milliseconds(0));

    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.bindLoader(&loader);
    panel.bindHotLoader(&hotLoader);

    REQUIRE(panel.registerManifest(manifest));
    REQUIRE(panel.activateMod(manifest.id));
    REQUIRE(panel.trackHotLoadMod(manifest.id));

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["hot_loader_bound"] == true);
    REQUIRE(snapshot["actions"]["poll_hot_loader"] == true);
    REQUIRE(snapshot["hot_load"]["event_count"] == 1);
    REQUIRE(snapshot["hot_load"]["events"][0]["type"] == "registered");
    REQUIRE(snapshot["hot_load"]["events"][0]["mod_id"] == manifest.id);

    {
        std::ofstream script(scriptPath, std::ios::binary | std::ios::trunc);
        REQUIRE(script.is_open());
        script << "export function activate(context) { context.commands.register('editor.hot.echo', function(v) { return 'v2:' + v; }); }";
    }

    const auto poll = panel.pollHotLoader();
    REQUIRE(poll.anyReloaded);

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["last_action"]["action"] == "poll_hot_loader");
    REQUIRE(snapshot["last_action"]["any_reloaded"] == true);
    REQUIRE(snapshot["hot_load"]["last_poll_any_reloaded"] == true);
    REQUIRE(snapshot["hot_load"]["last_poll_event_count"] >= 2);
    REQUIRE(snapshot["hot_load"]["event_count"] >= 3);
    REQUIRE(std::find_if(snapshot["hot_load"]["events"].begin(), snapshot["hot_load"]["events"].end(),
                         [](const nlohmann::json& event) { return event["type"] == "reloaded"; }) !=
            snapshot["hot_load"]["events"].end());

    urpg::Value payload;
    payload.v = std::string("ok");
    const auto commandResult = loader.executeCommand(manifest.id, "editor.hot.echo", {payload});
    REQUIRE(commandResult.has_value());
    REQUIRE(std::get<std::string>(commandResult->v) == "v2:ok");

    panel.clearHotLoadEventLog();
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["hot_load"]["event_count"] == 0);
    REQUIRE(snapshot["hot_load"]["last_poll_event_count"] == 0);

    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);
}

TEST_CASE("ModManagerPanel: Store catalog entries install through the editor surface",
          "[mod][editor][panel][store]") {
    ModRegistry registry;
    ModLoader loader(registry);
    ModStoreCatalog catalog;

    nlohmann::json document = {
        {"version", 1},
        {"entries", nlohmann::json::array({
                        {
                            {"id", "store.ui"},
                            {"displayName", "Store UI"},
                            {"version", "1.0.0"},
                            {"publisher", "URPG"},
                            {"packageRoot", "mods/store_ui"},
                            {"verified", true},
                            {"tags", nlohmann::json::array({"ui"})},
                            {"manifest",
                             {{"id", "store_ui"},
                              {"name", "Store UI"},
                              {"version", "1.0.0"},
                              {"author", "URPG"},
                              {"dependencies", nlohmann::json::array()},
                              {"entryPoint", "main.js"}}},
                            {"sandboxPolicy",
                             {{"fileSystemRead", true},
                              {"fileSystemWrite", false},
                              {"networkAccess", false},
                              {"nativeInterop", false}}},
                        },
                    })},
    };
    REQUIRE(catalog.loadFromJson(document));

    ModManagerPanel panel;
    panel.bindRegistry(&registry);
    panel.bindLoader(&loader);
    panel.bindStoreCatalog(&catalog);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["store_catalog_bound"] == true);
    REQUIRE(snapshot["store"]["entry_count"] == 1);
    REQUIRE(snapshot["store"]["entries"][0]["id"] == "store.ui");
    REQUIRE(snapshot["actions"]["install_store_entry"] == true);

    REQUIRE(panel.installStoreEntry("store.ui", "content/fixtures"));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["last_action"]["action"] == "install_store_entry");
    REQUIRE(snapshot["last_action"]["success"] == true);
    REQUIRE(snapshot["last_action"]["mod_id"] == "store_ui");
    REQUIRE(snapshot["registered_count"] == 1);
    REQUIRE(snapshot["mods"][0]["id"] == "store_ui");
    REQUIRE(snapshot["mods"][0]["sandbox_policy"]["file_system_read"] == "allowed");
}
