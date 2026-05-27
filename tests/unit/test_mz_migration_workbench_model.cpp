#include "editor/compat/mz_migration_workbench_model.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ migration workbench combines compatibility evidence into creator snapshot",
          "[editor][compat][mz_workbench]") {
    urpg::plugin::PluginCompatibilityReport plugin_report;
    urpg::plugin::PluginCompatibilityResult window_tweaks;
    window_tweaks.plugin_id = "WindowTweaks";
    window_tweaks.name = "Window Tweaks";
    window_tweaks.score = 55;
    window_tweaks.estimated_repair_minutes = 120;
    window_tweaks.confidence = "low";
    plugin_report.plugins.push_back(window_tweaks);

    urpg::plugin::PluginCompatibilityResult battle_hud;
    battle_hud.plugin_id = "BattleHud";
    battle_hud.name = "Battle HUD";
    battle_hud.score = 80;
    battle_hud.estimated_repair_minutes = 30;
    battle_hud.confidence = "medium";
    plugin_report.plugins.push_back(battle_hud);

    urpg::compat::MzProjectCompatibilityInput project_input;
    project_input.maps = {.score = 100, .covered_count = 4, .unsupported_count = 0, .id = "maps"};
    project_input.events = {.score = 70, .covered_count = 7, .unsupported_count = 3, .id = "events"};
    project_input.plugins = {.score = 65, .covered_count = 1, .unsupported_count = 1, .id = "plugins"};
    project_input.saves = {.score = 90, .covered_count = 3, .unsupported_count = 0, .id = "saves"};
    project_input.assets = {.score = 100, .covered_count = 12, .unsupported_count = 0, .id = "assets"};
    project_input.unsupported_event_command_count = 3;
    const auto project_report = urpg::compat::BuildMzProjectCompatibilityReport(project_input);

    const auto command_report = urpg::compat::AnalyzeMzEventCommandCoverage(
        {"show_text", "transfer_player", "plugin_command", "script_eval", "battle_processing"});

    auto visual_report = urpg::compat::BuildEmptyMzVisualDiffReport();
    visual_report.addComparison({"title", "sha256:mz-title", "sha256:urpg-title", 0.25});
    visual_report.addComparison({"menu", "sha256:mz-menu", "sha256:urpg-menu", 4.5});

    const auto snapshot = urpg::editor::BuildMzMigrationWorkbenchSnapshot(
        {.plugin_report = plugin_report,
         .project_report = project_report,
         .command_report = command_report,
         .visual_report = visual_report});

    REQUIRE(snapshot.project_score == project_report.project_score);
    REQUIRE(snapshot.manual_repair_minutes == 150);
    REQUIRE(snapshot.unsupported_event_command_count == 3);
    REQUIRE(snapshot.visual_diff_failed_scene_count == 1);
    REQUIRE_FALSE(snapshot.can_auto_migrate);
    REQUIRE(snapshot.low_confidence_plugin_count == 1);

    const auto json = snapshot.toJson();
    REQUIRE(json["project_score"] == project_report.project_score);
    REQUIRE(json["manual_repair_minutes"] == 150);
    REQUIRE(json["unsupported_event_command_count"] == 3);
    REQUIRE(json["visual_diff_status"] == "failed");
    REQUIRE(json["can_auto_migrate"] == false);
    REQUIRE(json["release_authoritative"] == false);
}

TEST_CASE("MZ migration workbench allows auto migration only for clean high-score evidence",
          "[editor][compat][mz_workbench]") {
    urpg::compat::MzProjectCompatibilityInput project_input;
    project_input.maps = {.score = 100, .covered_count = 4, .unsupported_count = 0, .id = "maps"};
    project_input.events = {.score = 95, .covered_count = 20, .unsupported_count = 0, .id = "events"};
    project_input.plugins = {.score = 100, .covered_count = 2, .unsupported_count = 0, .id = "plugins"};
    project_input.saves = {.score = 95, .covered_count = 3, .unsupported_count = 0, .id = "saves"};
    project_input.assets = {.score = 100, .covered_count = 12, .unsupported_count = 0, .id = "assets"};

    auto visual_report = urpg::compat::BuildEmptyMzVisualDiffReport();
    visual_report.addComparison({"title", "sha256:mz-title", "sha256:urpg-title", 0.25});

    const auto snapshot = urpg::editor::BuildMzMigrationWorkbenchSnapshot(
        {.plugin_report = {},
         .project_report = urpg::compat::BuildMzProjectCompatibilityReport(project_input),
         .command_report = urpg::compat::AnalyzeMzEventCommandCoverage({"show_text", "transfer_player"}),
         .visual_report = visual_report});

    REQUIRE(snapshot.project_score >= 90);
    REQUIRE(snapshot.manual_repair_minutes == 0);
    REQUIRE(snapshot.unsupported_event_command_count == 0);
    REQUIRE(snapshot.can_auto_migrate);
}
