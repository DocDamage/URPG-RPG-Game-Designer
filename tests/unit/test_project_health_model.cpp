#include "editor/diagnostics/project_health_model.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("ProjectHealthModel groups issues and orders fix queue deterministically",
          "[editor][diagnostics][project_health]") {
    urpg::editor::ProjectHealthModel model;

    model.ingestAuditReport(nlohmann::json{
        {"headline", "Health"},
        {"summary", "Actionable health queue."},
        {"statusDate", "2026-04-24"},
        {"readinessDate", "2026-04-19"},
        {"releaseBlockerCount", 1},
        {"exportBlockerCount", 1},
        {"issues", {
            {
                {"code", "z_warning"},
                {"title", "Warning"},
                {"detail", "General warning"},
                {"severity", "warning"},
                {"subsystem", "ui"}
            },
            {
                {"code", "a_export"},
                {"title", "Export bundle missing"},
                {"detail", "Export blocker"},
                {"severity", "error"},
                {"blocksExport", true},
                {"owningSubsystem", "export"},
                {"affectedPaths", {"exports/project_audit/export_manifest.json"}},
                {"validationCommands", {"powershell -ExecutionPolicy Bypass -File .\\tools\\ci\\check_release_readiness.ps1"}},
                {"acceptanceCriteria", {"Export manifest validates."}}
            },
            {
                {"code", "b_release"},
                {"title", "Release blocker"},
                {"detail", "Release blocker"},
                {"severity", "error"},
                {"blocksRelease", true},
                {"owningSubsystem", "battle_core"}
            }
        }}
    });

    const auto& snapshot = model.snapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.stale);
    REQUIRE(snapshot.status_date == "2026-04-24");
    REQUIRE(snapshot.readiness_date == "2026-04-19");
    REQUIRE(snapshot.release_blocker_count == 1);
    REQUIRE(snapshot.export_blocker_count == 1);
    REQUIRE(snapshot.fix_next.size() == 3);
    REQUIRE(snapshot.fix_next[0].code == "b_release");
    REQUIRE(snapshot.fix_next[1].code == "a_export");
    REQUIRE(snapshot.fix_next[2].code == "z_warning");
    REQUIRE(snapshot.fix_next[1].validation_commands[0] ==
            "powershell -ExecutionPolicy Bypass -File .\\tools\\ci\\check_release_readiness.ps1");

    REQUIRE(snapshot.groups.size() == 3);
    REQUIRE(snapshot.groups[0].group == urpg::editor::ProjectHealthGroup::ReleaseBlockers);
    REQUIRE(snapshot.groups[1].group == urpg::editor::ProjectHealthGroup::ExportBlockers);
    REQUIRE(snapshot.groups[2].group == urpg::editor::ProjectHealthGroup::Warnings);
}

TEST_CASE("ProjectHealthModel derives sparse report health without crashing",
          "[editor][diagnostics][project_health]") {
    urpg::editor::ProjectHealthModel model;

    model.ingestAuditReport(nlohmann::json{
        {"headline", "Sparse health"},
        {"releaseBlockerCount", 2},
        {"exportBlockerCount", 1},
        {"assetGovernanceIssueCount", 3},
        {"schemaGovernanceIssueCount", 1},
        {"projectArtifactIssueCount", 4},
        {"issues", {
            {
                {"code", "unknown_severity"},
                {"title", "Unknown severity maps to info"},
                {"severity", "critical"}
            }
        }}
    });

    const auto& snapshot = model.snapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE_FALSE(snapshot.stale);
    REQUIRE(snapshot.release_blocker_count == 2);
    REQUIRE(snapshot.export_blocker_count == 1);
    REQUIRE(snapshot.fix_next.size() == 6);
    REQUIRE(snapshot.fix_next.back().code == "unknown_severity");
    REQUIRE(snapshot.fix_next.back().severity == urpg::editor::ProjectHealthSeverity::Info);
}

TEST_CASE("ProjectHealthModel clear resets to empty state", "[editor][diagnostics][project_health]") {
    urpg::editor::ProjectHealthModel model;
    model.ingestAuditReport(nlohmann::json{{"headline", "Loaded"}, {"issues", nlohmann::json::array()}});

    model.clear();

    REQUIRE_FALSE(model.hasReportData());
    REQUIRE_FALSE(model.snapshot().has_data);
    REQUIRE(model.snapshot().fix_next.empty());
}
