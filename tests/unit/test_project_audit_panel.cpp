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
        {"localizationEvidenceIssueCount", 1},
        {"accessibilityArtifactIssueCount", 1},
        {"audioArtifactIssueCount", 2},
        {"characterArtifactIssueCount", 1},
        {"modArtifactIssueCount", 2},
        {"analyticsArtifactIssueCount", 4},
        {"performanceArtifactIssueCount", 3},
        {"releaseSignoffWorkflowIssueCount", 0},
        {"signoffArtifactIssueCount", 1},
        {"templateSpecArtifactIssueCount", 1},
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
            {"localizationEvidence", {
                {"path", "imports/reports/localization/localization_consistency_report.json"},
                {"available", true},
                {"enabled", true},
                {"usable", true},
                {"dependency", "template localization coverage evidence"},
                {"summary", "Checking canonical localization consistency evidence for selected template jrpg."},
                {"status", "missing_keys"},
                {"issueCount", 1},
                {"hasBundles", true},
                {"bundleCount", 2},
                {"missingLocaleCount", 1},
                {"missingKeyCount", 2},
                {"extraKeyCount", 1},
                {"masterLocale", "en"},
                {"bundles", {
                    {
                        {"path", "content/localization/en.json"},
                        {"locale", "en"},
                        {"keyCount", 3}
                    },
                    {
                        {"path", "content/localization/fr.json"},
                        {"locale", "fr"},
                        {"keyCount", 2}
                    }
                }}
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
            {"characterArtifacts", {
                {"path", "content/schemas/character_identity.schema.json"},
                {"available", true},
                {"issueCount", 1}
            }},
            {"modArtifacts", {
                {"path", "content/schemas/mod_manifest.schema.json"},
                {"available", true},
                {"issueCount", 2}
            }},
            {"analyticsArtifacts", {
                {"path", "content/schemas/analytics_config.schema.json"},
                {"available", true},
                {"issueCount", 4}
            }},
            {"performanceArtifacts", {
                {"path", "docs/presentation/performance_budgets.md"},
                {"available", true},
                {"issueCount", 3}
            }},
            {"releaseSignoffWorkflow", {
                {"path", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                {"available", true},
                {"issueCount", 0}
            }},
            {"signoffArtifacts", {
                {"enabled", true},
                {"dependency", "human-review-gated subsystem signoff artifacts"},
                {"summary", "Checking required subsystem signoff artifacts, conservative wording, and structured human-review signoff contracts for governed lanes."},
                {"available", true},
                {"issueCount", 1},
                {"expectedArtifacts", {
                    {
                        {"subsystemId", "battle_core"},
                        {"title", "Battle Core signoff"},
                        {"path", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                        {"required", true},
                        {"exists", true},
                        {"isRegularFile", true},
                        {"status", "contract_mismatch"},
                        {"wordingOk", true},
                        {"signoffContract", {
                            {"required", true},
                            {"artifactPath", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                            {"promotionRequiresHumanReview", false},
                            {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                            {"contractOk", false}
                        }}
                    }
                }}
            }},
            {"templateSpecArtifacts", {
                {"enabled", true},
                {"path", "docs/templates/jrpg_spec.md"},
                {"available", true},
                {"issueCount", 1},
                {"expectedArtifacts", {
                    {
                        {"path", "docs/templates/jrpg_spec.md"},
                        {"status", "parity_mismatch"},
                        {"templateIdMatches", true},
                        {"requiredSubsystemsMatch", false},
                        {"barsMatch", false},
                        {"missingRequiredSubsystems", {"save_data_core"}},
                        {"unexpectedRequiredSubsystems", {"2_5d_mode"}},
                        {"barMismatches", {
                            {
                                {"bar", "accessibility"},
                                {"label", "Accessibility"},
                                {"expectedStatus", "PARTIAL"},
                                {"specStatus", "PLANNED"}
                            }
                        }}
                    }
                }}
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
    REQUIRE(snapshot.localization_evidence_issue_count);
    REQUIRE(snapshot.accessibility_artifact_issue_count);
    REQUIRE(snapshot.audio_artifact_issue_count);
    REQUIRE(snapshot.character_artifact_issue_count);
    REQUIRE(snapshot.mod_artifact_issue_count);
    REQUIRE(snapshot.analytics_artifact_issue_count);
    REQUIRE(snapshot.performance_artifact_issue_count);
    REQUIRE(snapshot.release_signoff_workflow_issue_count);
    REQUIRE(snapshot.signoff_artifact_issue_count);
    REQUIRE(snapshot.template_spec_artifact_issue_count);
    REQUIRE(*snapshot.asset_governance_issue_count == 3);
    REQUIRE(*snapshot.schema_governance_issue_count == 2);
    REQUIRE(*snapshot.project_artifact_issue_count == 4);
    REQUIRE(*snapshot.localization_evidence_issue_count == 1);
    REQUIRE(*snapshot.accessibility_artifact_issue_count == 1);
    REQUIRE(*snapshot.audio_artifact_issue_count == 2);
    REQUIRE(*snapshot.character_artifact_issue_count == 1);
    REQUIRE(*snapshot.mod_artifact_issue_count == 2);
    REQUIRE(*snapshot.analytics_artifact_issue_count == 4);
    REQUIRE(*snapshot.performance_artifact_issue_count == 3);
    REQUIRE(*snapshot.release_signoff_workflow_issue_count == 0);
    REQUIRE(*snapshot.signoff_artifact_issue_count == 1);
    REQUIRE(*snapshot.template_spec_artifact_issue_count == 1);

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

    REQUIRE(snapshot.localization_evidence.has_value());
    REQUIRE(snapshot.localization_evidence->path == "imports/reports/localization/localization_consistency_report.json");
    REQUIRE(snapshot.localization_evidence->available.has_value());
    REQUIRE(*snapshot.localization_evidence->available);
    REQUIRE(snapshot.localization_evidence->enabled.has_value());
    REQUIRE(*snapshot.localization_evidence->enabled);
    REQUIRE(snapshot.localization_evidence->usable.has_value());
    REQUIRE(*snapshot.localization_evidence->usable);
    REQUIRE(snapshot.localization_evidence->dependency == "template localization coverage evidence");
    REQUIRE(snapshot.localization_evidence->summary ==
            "Checking canonical localization consistency evidence for selected template jrpg.");
    REQUIRE(snapshot.localization_evidence->status == "missing_keys");
    REQUIRE(snapshot.localization_evidence->issue_count.has_value());
    REQUIRE(*snapshot.localization_evidence->issue_count == 1);
    REQUIRE(snapshot.localization_evidence->has_bundles.has_value());
    REQUIRE(*snapshot.localization_evidence->has_bundles);
    REQUIRE(snapshot.localization_evidence->bundle_count == 2);
    REQUIRE(snapshot.localization_evidence->missing_locale_count == 1);
    REQUIRE(snapshot.localization_evidence->missing_key_count == 2);
    REQUIRE(snapshot.localization_evidence->extra_key_count == 1);
    REQUIRE(snapshot.localization_evidence->master_locale == "en");
    REQUIRE(snapshot.localization_evidence->bundles.has_value());
    REQUIRE(snapshot.localization_evidence->bundles->is_array());
    REQUIRE((*snapshot.localization_evidence->bundles)[0]["locale"] == "en");

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

    REQUIRE(snapshot.character_artifacts.has_value());
    REQUIRE(snapshot.character_artifacts->path == "content/schemas/character_identity.schema.json");
    REQUIRE(snapshot.character_artifacts->available.has_value());
    REQUIRE(*snapshot.character_artifacts->available);
    REQUIRE(snapshot.character_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.character_artifacts->issue_count == 1);

    REQUIRE(snapshot.mod_artifacts.has_value());
    REQUIRE(snapshot.mod_artifacts->path == "content/schemas/mod_manifest.schema.json");
    REQUIRE(snapshot.mod_artifacts->available.has_value());
    REQUIRE(*snapshot.mod_artifacts->available);
    REQUIRE(snapshot.mod_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.mod_artifacts->issue_count == 2);

    REQUIRE(snapshot.analytics_artifacts.has_value());
    REQUIRE(snapshot.analytics_artifacts->path == "content/schemas/analytics_config.schema.json");
    REQUIRE(snapshot.analytics_artifacts->available.has_value());
    REQUIRE(*snapshot.analytics_artifacts->available);
    REQUIRE(snapshot.analytics_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.analytics_artifacts->issue_count == 4);

    REQUIRE(snapshot.performance_artifacts.has_value());
    REQUIRE(snapshot.performance_artifacts->path == "docs/presentation/performance_budgets.md");
    REQUIRE(snapshot.performance_artifacts->available.has_value());
    REQUIRE(*snapshot.performance_artifacts->available);
    REQUIRE(snapshot.performance_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.performance_artifacts->issue_count == 3);

    REQUIRE(snapshot.release_signoff_workflow.has_value());
    REQUIRE(snapshot.release_signoff_workflow->path == "docs/RELEASE_SIGNOFF_WORKFLOW.md");
    REQUIRE(snapshot.release_signoff_workflow->available.has_value());
    REQUIRE(*snapshot.release_signoff_workflow->available);
    REQUIRE(snapshot.release_signoff_workflow->issue_count.has_value());
    REQUIRE(*snapshot.release_signoff_workflow->issue_count == 0);

    REQUIRE(snapshot.signoff_artifacts.has_value());
    REQUIRE(snapshot.signoff_artifacts->enabled.has_value());
    REQUIRE(*snapshot.signoff_artifacts->enabled);
    REQUIRE(snapshot.signoff_artifacts->dependency == "human-review-gated subsystem signoff artifacts");
    REQUIRE(snapshot.signoff_artifacts->summary ==
            "Checking required subsystem signoff artifacts, conservative wording, and structured human-review signoff contracts for governed lanes.");
    REQUIRE(snapshot.signoff_artifacts->available.has_value());
    REQUIRE(*snapshot.signoff_artifacts->available);
    REQUIRE(snapshot.signoff_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.signoff_artifacts->issue_count == 1);
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts.size() == 1);
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts[0].subsystem_id == "battle_core");
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts[0].status == "contract_mismatch");
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts[0].signoff_contract.has_value());
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts[0].signoff_contract->contract_ok == false);
    REQUIRE(snapshot.signoff_artifacts->expected_artifacts[0].signoff_contract->artifact_path ==
            "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md");

    REQUIRE(snapshot.template_spec_artifacts.has_value());
    REQUIRE(snapshot.template_spec_artifacts->enabled.has_value());
    REQUIRE(*snapshot.template_spec_artifacts->enabled);
    REQUIRE(snapshot.template_spec_artifacts->path == "docs/templates/jrpg_spec.md");
    REQUIRE(snapshot.template_spec_artifacts->available.has_value());
    REQUIRE(*snapshot.template_spec_artifacts->available);
    REQUIRE(snapshot.template_spec_artifacts->issue_count.has_value());
    REQUIRE(*snapshot.template_spec_artifacts->issue_count == 1);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts.size() == 1);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].status == "parity_mismatch");
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].required_subsystems_match == false);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].bars_match == false);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].missing_required_subsystems.size() == 1);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].missing_required_subsystems[0] == "save_data_core");
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].unexpected_required_subsystems.size() == 1);
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].unexpected_required_subsystems[0] == "2_5d_mode");
    REQUIRE(snapshot.template_spec_artifacts->expected_artifacts[0].bar_mismatches.has_value());
    REQUIRE((*snapshot.template_spec_artifacts->expected_artifacts[0].bar_mismatches)[0]["bar"] == "accessibility");
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
