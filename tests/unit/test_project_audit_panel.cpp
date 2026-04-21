#include "editor/diagnostics/project_audit_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("ProjectAuditPanel renders report snapshots from bound JSON", "[editor][diagnostics][project_audit]") {
    urpg::editor::ProjectAuditPanel panel;

    panel.setReportJson(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Audit headline"},
        {"summary", "Audit summary"},
        {"releaseBlockerCount", 2},
        {"exportBlockerCount", 1},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
        {"issues", {
            {
                {"code", "missing_localization_key"},
                {"title", "Missing localization key"},
                {"detail", "menu.start missing"},
                {"severity", "warning"},
                {"blocksRelease", true},
                {"blocksExport", false}
            },
            {
                {"code", "missing_input_binding"},
                {"title", "Missing input binding"},
                {"detail", "confirm action missing"},
                {"severity", "error"},
                {"blocksRelease", false},
                {"blocksExport", true}
            }
        }}
    });

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.hasReportData());
    REQUIRE(panel.currentIssueCount() == 2);
    REQUIRE(panel.currentReleaseBlockerCount() == 2);
    REQUIRE(panel.currentExportBlockerCount() == 1);

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.headline == "Audit headline");
    REQUIRE(snapshot.summary == "Audit summary");
    REQUIRE(snapshot.issue_count == 2);
    REQUIRE(snapshot.release_blocker_count == 2);
    REQUIRE(snapshot.export_blocker_count == 1);
    REQUIRE(snapshot.template_id == "jrpg");
    REQUIRE(snapshot.template_status == "PARTIAL");
    REQUIRE(snapshot.issues.size() == 2);
    REQUIRE(snapshot.issues[0].code == "missing_localization_key");
    REQUIRE(snapshot.issues[0].severity == urpg::editor::ProjectAuditSeverity::Warning);
    REQUIRE(snapshot.issues[0].blocks_release);
    REQUIRE(snapshot.issues[1].code == "missing_input_binding");
    REQUIRE(snapshot.issues[1].severity == urpg::editor::ProjectAuditSeverity::Error);
    REQUIRE(snapshot.issues[1].blocks_export);
}

TEST_CASE("ProjectAuditPanel captures governance details when present", "[editor][diagnostics][project_audit]") {
    urpg::editor::ProjectAuditPanel panel;

    panel.setReportJson(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Governance headline"},
        {"summary", "Governance summary"},
        {"releaseBlockerCount", 1},
        {"exportBlockerCount", 0},
        {"assetGovernanceIssueCount", 3},
        {"schemaGovernanceIssueCount", 2},
        {"projectArtifactIssueCount", 4},
        {"accessibilityArtifactIssueCount", 1},
        {"audioArtifactIssueCount", 2},
        {"performanceArtifactIssueCount", 3},
        {"templateContext", {{"id", "jrpg"}, {"status", "READY"}}},
        {"governance", {
            {"assetReport", {
                {"path", "imports/reports/asset_intake/source_capture_status.json"},
                {"available", true},
                {"usable", false},
                {"issueCount", 3}
            }},
            {"schema", {
                {"schemaExists", true},
                {"changelogExists", false},
                {"mentionsSchemaVersion", true},
                {"schemaVersion", "1.0.0"}
            }},
            {"projectSchema", {
                {"path", "content/schemas/project.schema.json"},
                {"available", true},
                {"hasLocalizationSection", true},
                {"hasInputSection", false},
                {"hasExportSection", true}
            }},
            {"localizationArtifacts", {
                {"path", "content/localization/en-US.json"},
                {"available", true},
                {"issueCount", 1}
            }},
            {"inputArtifacts", {
                {"path", "content/input/input_bindings.json"},
                {"exists", true},
                {"issueCount", 2}
            }},
            {"exportArtifacts", {
                {"path", "exports/project_audit/export_manifest.json"},
                {"present", true},
                {"issueCount", 2}
            }},
            {"accessibilityArtifacts", {
                {"path", "content/schemas/accessibility_report.schema.json"},
                {"available", true},
                {"issueCount", 1}
            }},
            {"audioArtifacts", {
                {"path", "content/schemas/audio_mix_presets.schema.json"},
                {"available", true},
                {"issueCount", 2}
            }},
            {"performanceArtifacts", {
                {"path", "docs/presentation/performance_budgets.md"},
                {"available", true},
                {"issueCount", 3}
            }}
        }},
        {"issues", nlohmann::json::array()}
    });

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.hasReportData());

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.asset_governance_issue_count);
    REQUIRE(snapshot.schema_governance_issue_count);
    REQUIRE(snapshot.project_artifact_issue_count);
    REQUIRE(snapshot.accessibility_artifact_issue_count);
    REQUIRE(snapshot.audio_artifact_issue_count);
    REQUIRE(snapshot.performance_artifact_issue_count);
    REQUIRE(*snapshot.asset_governance_issue_count == 3);
    REQUIRE(*snapshot.schema_governance_issue_count == 2);
    REQUIRE(*snapshot.project_artifact_issue_count == 4);
    REQUIRE(*snapshot.accessibility_artifact_issue_count == 1);
    REQUIRE(*snapshot.audio_artifact_issue_count == 2);
    REQUIRE(*snapshot.performance_artifact_issue_count == 3);

    REQUIRE(snapshot.asset_report.has_value());
    REQUIRE(snapshot.asset_report->path == "imports/reports/asset_intake/source_capture_status.json");
    REQUIRE(snapshot.asset_report->available.has_value());
    REQUIRE(*snapshot.asset_report->available);
    REQUIRE(snapshot.asset_report->usable.has_value());
    REQUIRE_FALSE(*snapshot.asset_report->usable);
    REQUIRE(snapshot.asset_report->issue_count == 3);

    REQUIRE(snapshot.schema_governance.has_value());
    REQUIRE(snapshot.schema_governance->schema_exists.has_value());
    REQUIRE(*snapshot.schema_governance->schema_exists);
    REQUIRE(snapshot.schema_governance->changelog_exists.has_value());
    REQUIRE_FALSE(*snapshot.schema_governance->changelog_exists);
    REQUIRE(snapshot.schema_governance->mentions_schema_version.has_value());
    REQUIRE(*snapshot.schema_governance->mentions_schema_version);
    REQUIRE(snapshot.schema_governance->schema_version == "1.0.0");

    REQUIRE(snapshot.project_schema_governance.has_value());
    REQUIRE(snapshot.project_schema_governance->path == "content/schemas/project.schema.json");
    REQUIRE(snapshot.project_schema_governance->available.has_value());
    REQUIRE(*snapshot.project_schema_governance->available);
    REQUIRE(snapshot.project_schema_governance->has_localization_section.has_value());
    REQUIRE(*snapshot.project_schema_governance->has_localization_section);
    REQUIRE(snapshot.project_schema_governance->has_input_section.has_value());
    REQUIRE_FALSE(*snapshot.project_schema_governance->has_input_section);
    REQUIRE(snapshot.project_schema_governance->has_export_section.has_value());
    REQUIRE(*snapshot.project_schema_governance->has_export_section);

    REQUIRE(snapshot.localization_artifacts.has_value());
    REQUIRE(snapshot.localization_artifacts->path == "content/localization/en-US.json");
    REQUIRE(snapshot.localization_artifacts->available.has_value());
    REQUIRE(*snapshot.localization_artifacts->available);
    REQUIRE(snapshot.localization_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.localization_artifacts->issue_count == 1);

    REQUIRE(snapshot.input_artifacts.has_value());
    REQUIRE(snapshot.input_artifacts->path == "content/input/input_bindings.json");
    REQUIRE(snapshot.input_artifacts->available.has_value());
    REQUIRE(*snapshot.input_artifacts->available);
    REQUIRE(snapshot.input_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.input_artifacts->issue_count == 2);

    REQUIRE(snapshot.export_artifacts.has_value());
    REQUIRE(snapshot.export_artifacts->path == "exports/project_audit/export_manifest.json");
    REQUIRE(snapshot.export_artifacts->available.has_value());
    REQUIRE(*snapshot.export_artifacts->available);
    REQUIRE(snapshot.export_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.export_artifacts->issue_count == 2);

    REQUIRE(snapshot.accessibility_artifacts.has_value());
    REQUIRE(snapshot.accessibility_artifacts->path == "content/schemas/accessibility_report.schema.json");
    REQUIRE(snapshot.accessibility_artifacts->available.has_value());
    REQUIRE(*snapshot.accessibility_artifacts->available);
    REQUIRE(snapshot.accessibility_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.accessibility_artifacts->issue_count == 1);

    REQUIRE(snapshot.audio_artifacts.has_value());
    REQUIRE(snapshot.audio_artifacts->path == "content/schemas/audio_mix_presets.schema.json");
    REQUIRE(snapshot.audio_artifacts->available.has_value());
    REQUIRE(*snapshot.audio_artifacts->available);
    REQUIRE(snapshot.audio_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.audio_artifacts->issue_count == 2);

    REQUIRE(snapshot.performance_artifacts.has_value());
    REQUIRE(snapshot.performance_artifacts->path == "docs/presentation/performance_budgets.md");
    REQUIRE(snapshot.performance_artifacts->available.has_value());
    REQUIRE(*snapshot.performance_artifacts->available);
    REQUIRE(snapshot.performance_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.performance_artifacts->issue_count == 3);
}

TEST_CASE("ProjectAuditPanel derives blocker counts from issue flags when report counts are absent",
          "[editor][diagnostics][project_audit]") {
    urpg::editor::ProjectAuditPanel panel;

    panel.setReportJson(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Derived counts"},
        {"summary", "Counts should come from issue flags when the report omits totals."},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
        {"issues", {
            {
                {"code", "missing_localization_key"},
                {"title", "Missing localization key"},
                {"blocksRelease", true}
            },
            {
                {"code", "missing_input_binding"},
                {"title", "Missing input binding"},
                {"blocksExport", true}
            },
            {
                {"code", "missing_particles"},
                {"title", "Missing particles"},
                {"blocksRelease", true},
                {"blocksExport", true}
            },
            {
                {"code", "informational_note"},
                {"title", "Informational note"}
            }
        }}
    });

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.hasReportData());
    REQUIRE(panel.currentIssueCount() == 4);
    REQUIRE(panel.currentReleaseBlockerCount() == 2);
    REQUIRE(panel.currentExportBlockerCount() == 2);

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.issue_count == 4);
    REQUIRE(snapshot.release_blocker_count == 2);
    REQUIRE(snapshot.export_blocker_count == 2);
    REQUIRE(snapshot.issues.size() == 4);
    REQUIRE(snapshot.issues[0].blocks_release);
    REQUIRE(snapshot.issues[1].blocks_export);
    REQUIRE(snapshot.issues[2].blocks_release);
    REQUIRE(snapshot.issues[2].blocks_export);
    REQUIRE(snapshot.template_id == "jrpg");
    REQUIRE(snapshot.template_status == "PARTIAL");
}

TEST_CASE("ProjectAuditPanel renders safely without report data", "[editor][diagnostics][project_audit]") {
    urpg::editor::ProjectAuditPanel panel;

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.hasReportData());

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.has_data);
    REQUIRE(snapshot.issue_count == 0);
    REQUIRE(snapshot.release_blocker_count == 0);
    REQUIRE(snapshot.export_blocker_count == 0);
    REQUIRE(snapshot.issues.empty());
}

TEST_CASE("ProjectAuditPanel clear resets report state", "[editor][diagnostics][project_audit]") {
    urpg::editor::ProjectAuditPanel panel;
    panel.setReportJson(nlohmann::json{
        {"headline", "Audit headline"},
        {"issues", nlohmann::json::array()}
    });
    panel.render();

    panel.clear();

    REQUIRE_FALSE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.hasReportData());
    REQUIRE(panel.currentIssueCount() == 0);
    REQUIRE(panel.currentReleaseBlockerCount() == 0);
    REQUIRE(panel.currentExportBlockerCount() == 0);
}
