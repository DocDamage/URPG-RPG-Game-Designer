#include <catch2/catch_test_macros.hpp>

#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"

#include <chrono>
#include <filesystem>

namespace {

std::filesystem::path uniqueTempProjectRoot(const std::string& name) {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() / ("urpg_" + name + "_" + std::to_string(unique));
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

void startShellForDialogueCommands(urpg::EngineShell& shell, const std::filesystem::path& project_root) {
    if (shell.isRunning()) {
        shell.shutdown();
    }

    REQUIRE(shell.startup(std::make_unique<urpg::HeadlessSurface>(), std::make_unique<urpg::HeadlessRenderer>(),
                          urpg::EngineShell::StartupOptions(project_root)));
}

} // namespace

TEST_CASE("Dialogue commands mutate inventory health and quest state", "[message][dialogue_commands][integration]") {
    auto& shell = urpg::EngineShell::getInstance();
    const auto projectRoot = uniqueTempProjectRoot("dialogue_commands_success");
    startShellForDialogueCommands(shell, projectRoot);

    shell.configureDialogueQuest("intro", 3);

    auto result = shell.executeDialogueCommand("GIVE_ITEM:potion:2");
    REQUIRE(result.handled);
    REQUIRE(result.success);
    REQUIRE(result.code == "item_granted");

    result = shell.executeDialogueCommand("HEAL_PLAYER:20");
    REQUIRE(result.success);
    REQUIRE(result.code == "player_healed");

    result = shell.executeDialogueCommand("QUEST_ADVANCE:intro:3");
    REQUIRE(result.success);
    REQUIRE(result.code == "quest_advanced");

    const auto snapshot = shell.exportDialogueGameplaySnapshot();
    REQUIRE(snapshot["inventory"].size() == 1);
    REQUIRE(snapshot["inventory"][0]["item_id"] == "potion");
    REQUIRE(snapshot["inventory"][0]["count"] == 2);
    REQUIRE(snapshot["health"]["current"] == 95);
    REQUIRE(snapshot["health"]["max"] == 100);
    REQUIRE(snapshot["quest"]["id"] == "intro");
    REQUIRE(snapshot["quest"]["state"] == "completed");
    REQUIRE(snapshot["quest"]["current_progress"] == 3);

    const auto diagnostics = shell.exportDialogueCommandDiagnostics();
    REQUIRE(diagnostics.size() == 3);
    REQUIRE(diagnostics[0]["command"] == "GIVE_ITEM:potion:2");
    REQUIRE(diagnostics[2]["code"] == "quest_advanced");

    shell.shutdown();
    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("Dialogue command failures are structured and exported", "[message][dialogue_commands][integration]") {
    auto& shell = urpg::EngineShell::getInstance();
    const auto projectRoot = uniqueTempProjectRoot("dialogue_commands_failure");
    startShellForDialogueCommands(shell, projectRoot);

    auto result = shell.executeDialogueCommand("GIVE_ITEM:");
    REQUIRE(result.handled);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "invalid_item");

    result = shell.executeDialogueCommand("HEAL_PLAYER:not_number");
    REQUIRE(result.handled);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "invalid_heal_amount");

    result = shell.executeDialogueCommand("QUEST_ADVANCE:missing:1");
    REQUIRE(result.handled);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "quest_not_active");

    result = shell.executeDialogueCommand("UNKNOWN_COMMAND:anything");
    REQUIRE_FALSE(result.handled);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "unknown_command");

    const auto diagnostics = shell.exportDialogueCommandDiagnostics();
    REQUIRE(diagnostics.size() == 4);
    REQUIRE(diagnostics[0]["success"] == false);
    REQUIRE(diagnostics[1]["code"] == "invalid_heal_amount");
    REQUIRE(diagnostics[2]["message"].get<std::string>().find("Quest") != std::string::npos);
    REQUIRE(diagnostics[3]["handled"] == false);

    shell.shutdown();
    std::filesystem::remove_all(projectRoot);
}
