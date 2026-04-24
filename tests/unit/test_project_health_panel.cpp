#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/project_health_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("ProjectHealthPanel renders no-data and loaded snapshots",
          "[editor][diagnostics][project_health]") {
    urpg::editor::ProjectHealthPanel panel;

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.hasReportData());
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);

    panel.setReportJson(nlohmann::json{
        {"headline", "Health loaded"},
        {"issues", {
            {
                {"code", "release_gap"},
                {"title", "Release gap"},
                {"severity", "error"},
                {"blocksRelease", true}
            }
        }}
    });
    panel.render();

    REQUIRE(panel.hasReportData());
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().fix_next.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().groups.size() == 1);
}

TEST_CASE("DiagnosticsWorkspace exports project health fix queue",
          "[editor][diagnostics][project_health]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindProjectAuditReport(nlohmann::json{
        {"headline", "Workspace health"},
        {"summary", "Fix next queue"},
        {"releaseBlockerCount", 1},
        {"issues", {
            {
                {"code", "battle_signoff"},
                {"title", "Battle signoff required"},
                {"detail", "Human review must close."},
                {"severity", "error"},
                {"blocksRelease", true},
                {"owningSubsystem", "battle_core"},
                {"validationCommands", {"powershell -ExecutionPolicy Bypass -File .\\tools\\ci\\truth_reconciler.ps1"}}
            }
        }}
    });
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::ProjectHealth);
    workspace.render();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "project_health");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["fix_next"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["fix_next"][0]["code"] == "battle_signoff");
    REQUIRE(exported["active_tab_detail"]["fix_next"][0]["validation_commands"][0] ==
            "powershell -ExecutionPolicy Bypass -File .\\tools\\ci\\truth_reconciler.ps1");
}
