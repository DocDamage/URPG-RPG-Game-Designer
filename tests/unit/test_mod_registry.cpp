#include <catch2/catch_test_macros.hpp>

#include "engine/core/mod/mod_registry.h"
#include "engine/core/mod/mod_registry_validator.h"
#include "engine/core/mod/mod_loader.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace urpg::mod;
using nlohmann::json;

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

TEST_CASE("ModRegistryValidator: bounded manifest issues are reported", "[mod][validation]") {
    ModRegistryValidator validator;

    ModManifest invalid;
    invalid.id = "broken_mod";
    invalid.dependencies = {"broken_mod", "shared_dep", "shared_dep"};

    const auto issues = validator.validate({invalid});
    REQUIRE(issues.size() == 5);
    REQUIRE(issues[0].category == ModIssueCategory::EmptyName);
    REQUIRE(issues[1].category == ModIssueCategory::EmptyVersion);
    REQUIRE(issues[2].category == ModIssueCategory::MissingEntryPoint);
    REQUIRE(issues[3].category == ModIssueCategory::SelfDependency);
    REQUIRE(issues[4].category == ModIssueCategory::DuplicateDependency);
}

TEST_CASE("ModRegistryValidator: CI governance script validates artifacts", "[mod][validation][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_mod_governance.ps1";
    const auto uniqueSuffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto outputPath =
        std::filesystem::temp_directory_path() / ("urpg_mod_gov_out_" + uniqueSuffix + ".json");

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    const auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}

// ─── S28-T05/T06: ModLoader — live loading, sandboxing, store contract ────────

static ModManifest makeValidManifest(const std::string& id) {
    ModManifest m;
    m.id = id;
    m.name = "Mod " + id;
    m.version = "1.0.0";
    m.entryPoint = "scripts/" + id + ".js";
    return m;
}

TEST_CASE("ModLoader: loadMod registers and activates a valid mod",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    auto result = loader.loadMod(makeValidManifest("alpha"));

    REQUIRE(result.success);
    REQUIRE(result.modId == "alpha");
    REQUIRE(registry.getMod("alpha").has_value());
    const auto active = registry.listActiveMods();
    REQUIRE(std::find(active.begin(), active.end(), "alpha") != active.end());
}

TEST_CASE("ModLoader: loadMod stores sandbox policy and returns it via getSandboxPolicy",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    ModSandboxPolicy policy;
    policy.allowFileSystemRead = true;
    policy.allowNetworkAccess = false;

    auto result = loader.loadMod(makeValidManifest("beta"), policy);
    REQUIRE(result.success);

    auto stored = loader.getSandboxPolicy("beta");
    REQUIRE(stored.has_value());
    REQUIRE(stored->allowFileSystemRead == true);
    REQUIRE(stored->allowNetworkAccess == false);
    REQUIRE(stored->allowNativeInterop == false);
}

TEST_CASE("ModLoader: unloadMod removes mod, sandbox policy, and updates load order",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    loader.loadMod(makeValidManifest("gamma"));
    REQUIRE(registry.getMod("gamma").has_value());

    auto result = loader.unloadMod("gamma");
    REQUIRE(result.success);
    REQUIRE_FALSE(registry.getMod("gamma").has_value());
    REQUIRE_FALSE(loader.getSandboxPolicy("gamma").has_value());
    // No remaining mods → empty load order
    REQUIRE(result.loadOrder.empty());
}

TEST_CASE("ModLoader: unloadMod of unknown mod returns failure",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    auto result = loader.unloadMod("no_such_mod");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorMessage.find("Unknown mod") != std::string::npos);
}

TEST_CASE("ModLoader: loadMod returns deterministic load order across multiple mods",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    loader.loadMod(makeValidManifest("dep_a"));
    auto result = loader.loadMod(makeValidManifest("dep_b"));
    REQUIRE(result.success);
    // Both mods should appear in the resolved load order
    REQUIRE(result.loadOrder.size() == 2);
}

TEST_CASE("ModLoader: duplicate loadMod returns failure",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    REQUIRE(loader.loadMod(makeValidManifest("dup")).success);
    auto result = loader.loadMod(makeValidManifest("dup"));
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorMessage.find("already registered") != std::string::npos);
}

TEST_CASE("ModLoader: default sandbox policy is deny-all",
          "[mod][loader][s28t05]") {
    ModRegistry registry;
    ModLoader loader(registry);

    loader.loadMod(makeValidManifest("restricted"));
    auto policy = loader.getSandboxPolicy("restricted");
    REQUIRE(policy.has_value());
    REQUIRE(policy->allowFileSystemRead == false);
    REQUIRE(policy->allowFileSystemWrite == false);
    REQUIRE(policy->allowNetworkAccess == false);
    REQUIRE(policy->allowNativeInterop == false);
}

TEST_CASE("ModLoader: validateContract rejects missing id",
          "[mod][loader][s28t06]") {
    ModRegistry registry;
    ModLoader loader(registry);

    ModManifest bad = makeValidManifest("x");
    bad.id = "";

    auto failures = loader.validateContract(bad);
    REQUIRE_FALSE(failures.empty());
    REQUIRE(failures[0].field == "id");
}

TEST_CASE("ModLoader: validateContract rejects missing entryPoint",
          "[mod][loader][s28t06]") {
    ModRegistry registry;
    ModLoader loader(registry);

    ModManifest bad = makeValidManifest("y");
    bad.entryPoint = "";

    auto failures = loader.validateContract(bad);
    REQUIRE_FALSE(failures.empty());
    REQUIRE(failures[0].field == "entryPoint");
}

TEST_CASE("ModLoader: validateContract passes for well-formed manifest",
          "[mod][loader][s28t06]") {
    ModRegistry registry;
    ModLoader loader(registry);

    auto failures = loader.validateContract(makeValidManifest("good_mod"));
    REQUIRE(failures.empty());
}

TEST_CASE("ModLoader: loadMod with contract violation returns failure without registering",
          "[mod][loader][s28t06]") {
    ModRegistry registry;
    ModLoader loader(registry);

    ModManifest bad;
    bad.id = "bad";
    bad.name = "Bad Mod";
    // version and entryPoint intentionally empty

    auto result = loader.loadMod(bad);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorMessage.find("Contract violation") != std::string::npos);
    REQUIRE_FALSE(registry.getMod("bad").has_value());
}

TEST_CASE("ModLoader: setStoreContract adjusts required fields",
          "[mod][loader][s28t06]") {
    ModRegistry registry;
    ModLoader loader(registry);

    // Relax contract: entryPoint not required
    ModStoreContract relaxed;
    relaxed.requireEntryPoint = false;
    loader.setStoreContract(relaxed);

    ModManifest m = makeValidManifest("relaxed_mod");
    m.entryPoint = "";  // would fail with default contract

    auto result = loader.loadMod(m);
    REQUIRE(result.success);
}
