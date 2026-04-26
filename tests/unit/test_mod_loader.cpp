#include <catch2/catch_test_macros.hpp>

#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace urpg::mod;

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

std::filesystem::path sampleRoot() {
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        return sourceRoot / "content" / "fixtures" / "mod_sdk_sample";
    }
    return std::filesystem::current_path() / "content" / "fixtures" / "mod_sdk_sample";
}

ModManifest manifestFromJson(const nlohmann::json& manifestJson, const std::filesystem::path& root) {
    ModManifest manifest;
    manifest.id = manifestJson.value("id", "");
    manifest.name = manifestJson.value("name", "");
    manifest.version = manifestJson.value("version", "");
    manifest.author = manifestJson.value("author", "");
    manifest.entryPoint = (root / manifestJson.value("entryPoint", "")).string();
    if (const auto dependenciesIt = manifestJson.find("dependencies");
        dependenciesIt != manifestJson.end() && dependenciesIt->is_array()) {
        for (const auto& dependency : *dependenciesIt) {
            if (dependency.is_string()) {
                manifest.dependencies.push_back(dependency.get<std::string>());
            }
        }
    }
    return manifest;
}

} // namespace

TEST_CASE("ModLoader executes sample mod lifecycle through sandboxed QuickJS", "[mod]") {
    const auto root = sampleRoot();
    std::ifstream manifestInput(root / "manifest.json");
    REQUIRE(manifestInput.is_open());
    const auto manifestJson = nlohmann::json::parse(manifestInput);

    ModRegistry registry;
    ModLoader loader(registry);

    ModSandboxPolicy policy;
    policy.allowFileSystemRead = true;

    const ModManifest manifest = manifestFromJson(manifestJson, root);
    const auto loadResult = loader.loadMod(manifest, policy);
    REQUIRE(loadResult.success);
    REQUIRE(registry.listActiveMods().size() == 1);

    const auto snapshot = loader.getRuntimeSnapshot("urpg.sample.echo");
    REQUIRE(snapshot.active);
    REQUIRE(snapshot.scriptLoaded);
    REQUIRE(snapshot.commandIds.size() == 1);
    REQUIRE(snapshot.commandIds[0] == "sample.echo");
    REQUIRE(snapshot.diagnostics.size() == 1);
    REQUIRE(snapshot.diagnostics[0].severity == "info");
    REQUIRE(snapshot.diagnostics[0].code == "sample.echo.loaded");
    REQUIRE(snapshot.diagnostics[0].message == "Sample echo mod loaded.");

    urpg::Value payload;
    payload.v = std::string("runtime-state");
    const auto commandResult = loader.executeCommand("urpg.sample.echo", "sample.echo", {payload});
    REQUIRE(commandResult.has_value());
    REQUIRE(std::holds_alternative<std::string>(commandResult->v));
    REQUIRE(std::get<std::string>(commandResult->v) == "runtime-state");
}

TEST_CASE("ModLoader rolls back activation when sandbox denies requested source behavior", "[mod]") {
    const auto tempDir = std::filesystem::temp_directory_path() / "urpg_bad_mod_sandbox";
    std::filesystem::create_directories(tempDir);
    const auto scriptPath = tempDir / "bad_mod.js";
    {
        std::ofstream script(scriptPath, std::ios::binary);
        REQUIRE(script.is_open());
        script << "export function activate(context) { fetch('https://example.invalid'); }";
    }

    ModManifest manifest;
    manifest.id = "urpg.bad.network";
    manifest.name = "Bad Network Mod";
    manifest.version = "1.0.0";
    manifest.entryPoint = scriptPath.string();

    ModRegistry registry;
    ModLoader loader(registry);
    const auto result = loader.loadMod(manifest, ModSandboxPolicy{});

    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorMessage.find("network access is denied") != std::string::npos);
    REQUIRE(registry.listActiveMods().empty());

    const auto snapshot = loader.getRuntimeSnapshot("urpg.bad.network");
    REQUIRE_FALSE(snapshot.active);
    REQUIRE_FALSE(snapshot.scriptLoaded);

    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);
}
