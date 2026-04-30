#include "project_audit_cli_test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace urpg::tests::project_audit_cli;

TEST_CASE("Project audit CLI fails when --input is missing its path", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);

    const ProcessResult result = runProjectAudit({"--json", "--input"}, repoRoot);

    REQUIRE(result.exitCode != 0);
    REQUIRE(result.stderrText.find("Missing path after --input") != std::string::npos);
}

TEST_CASE("Project audit CLI checks canonical template spec artifacts for the selected template",
          "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_template_spec";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-20"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", json::object()},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "docs" / "templates" / "artifact_template_spec.md",
        "# Artifact Template Spec\n\nStatus Date: 2026-04-20\nAuthority: canonical template spec for `artifact_template`\n");

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["governance"].contains("templateSpecArtifacts"));
    REQUIRE(report["templateSpecArtifactIssueCount"] == 0);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["issueCount"] == 0);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["enabled"] == true);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"].is_array());
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"].size() == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["path"] ==
            (fs::path("docs") / "templates" / "artifact_template_spec.md").string());
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["status"] == "present");
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["templateIdMatches"] == true);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["requiredSubsystemsMatch"] == true);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["barsMatch"] == true);
}

TEST_CASE("Project audit CLI reports missing canonical template spec artifacts for the selected template",
          "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_template_spec_missing";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-20"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", json::object()},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["governance"].contains("templateSpecArtifacts"));
    REQUIRE(report["templateSpecArtifactIssueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["issueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"].is_array());
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"].size() == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["status"] == "missing");
    REQUIRE(report["summary"].get<std::string>().find("Template-spec artifact issues: 1.") != std::string::npos);
}

TEST_CASE("Project audit CLI exposes structured signoff contract state for governed subsystem artifacts",
          "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path readinessPath = repoRoot / "content" / "readiness" / "readiness_status.json";

    const ProcessResult result = runProjectAudit({"--json", "--input", readinessPath.string()}, repoRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["governance"].contains("signoffArtifacts"));
    REQUIRE(report["governance"]["signoffArtifacts"]["expectedArtifacts"].is_array());
    REQUIRE(report["governance"]["signoffArtifacts"]["expectedArtifacts"].size() == 3);

    const auto& battleArtifact = report["governance"]["signoffArtifacts"]["expectedArtifacts"][0];
    REQUIRE(battleArtifact["subsystemId"] == "battle_core");
    REQUIRE(battleArtifact.contains("signoffContract"));
    REQUIRE(battleArtifact["signoffContract"]["required"] == true);
    REQUIRE(battleArtifact["signoffContract"]["artifactPath"] == "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md");
    REQUIRE(battleArtifact["signoffContract"]["promotionRequiresHumanReview"] == false);
    REQUIRE(battleArtifact["signoffContract"]["reviewStatus"] == "APPROVED");
    REQUIRE(battleArtifact["signoffContract"]["workflow"] == "docs/RELEASE_SIGNOFF_WORKFLOW.md");
    REQUIRE(battleArtifact["signoffContract"]["contractOk"] == true);
}

TEST_CASE("Project audit CLI reports structured signoff contract drift for governed subsystem artifacts",
          "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_signoff_contract_drift";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-21"},
        {"subsystems", json::array({
            {
                {"id", "battle_core"},
                {"status", "PARTIAL"},
                {"summary", "Battle signoff exists; promotion to READY requires human review."},
                {"mainGaps", json::array({"Battle signoff exists; promotion to READY requires human review."})},
                {"signoff", {
                    {"required", true},
                    {"artifactPath", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                    {"promotionRequiresHumanReview", false},
                    {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"}
                }},
                {"evidence", {
                    {"runtimeOwner", true},
                    {"editorSurface", true},
                    {"schemaMigration", true},
                    {"diagnostics", true},
                    {"testsValidation", true},
                    {"docsAligned", true}
                }}
            },
            {
                {"id", "save_data_core"},
                {"status", "PARTIAL"},
                {"summary", "Save signoff exists; promotion to READY requires human review."},
                {"mainGaps", json::array({"Save signoff exists; promotion to READY requires human review."})},
                {"signoff", {
                    {"required", true},
                    {"artifactPath", "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"},
                    {"promotionRequiresHumanReview", true},
                    {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"}
                }},
                {"evidence", {
                    {"runtimeOwner", true},
                    {"editorSurface", true},
                    {"schemaMigration", true},
                    {"diagnostics", true},
                    {"testsValidation", true},
                    {"docsAligned", true}
                }}
            },
            {
                {"id", "compat_bridge_exit"},
                {"status", "PARTIAL"},
                {"summary", "Compat signoff exists; promotion to READY requires human review."},
                {"mainGaps", json::array({"Compat signoff exists; promotion to READY requires human review."})},
                {"signoff", {
                    {"required", true},
                    {"artifactPath", "docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md"},
                    {"promotionRequiresHumanReview", true},
                    {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"}
                }},
                {"evidence", {
                    {"runtimeOwner", true},
                    {"editorSurface", false},
                    {"schemaMigration", true},
                    {"diagnostics", true},
                    {"testsValidation", true},
                    {"docsAligned", true}
                }}
            }
        })},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", json::object()},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "docs" / "BATTLE_CORE_CLOSURE_SIGNOFF.md",
        "# Battle Core Signoff\n\nHuman review is required.\nresidual gaps remain.\nStatus: PARTIAL\n");
    writeTextFile(
        tempRoot / "docs" / "SAVE_DATA_CORE_CLOSURE_SIGNOFF.md",
        "# Save Data Core Signoff\n\nHuman review is required.\nresidual gaps remain.\nStatus: PARTIAL\n");
    writeTextFile(
        tempRoot / "docs" / "COMPAT_BRIDGE_EXIT_SIGNOFF.md",
        "# Compat Bridge Exit\n\nCompat Bridge Exit.\nHuman review is required.\ncompat bridge exit remains bounded.\nresidual gaps remain.\nStatus: PARTIAL\n");
    writeTextFile(
        tempRoot / "docs" / "RELEASE_SIGNOFF_WORKFLOW.md",
        "# Release Signoff Workflow\n\nThis is the canonical workflow.\nIt does not grant release approval.\nIn plain terms, it does not grant release approval.\nHuman review remains required.\nRun check_release_readiness.ps1 and truth_reconciler.ps1.\n");

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["signoffArtifactIssueCount"] == 1);
    REQUIRE(report["governance"]["signoffArtifacts"]["issueCount"] == 1);
    REQUIRE(report["governance"]["signoffArtifacts"]["expectedArtifacts"][0]["status"] == "contract_mismatch");
    REQUIRE(report["governance"]["signoffArtifacts"]["expectedArtifacts"][0]["signoffContract"]["contractOk"] == false);

    bool foundContractIssue = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"] == "signoff_artifact.battle_contract_mismatch") {
            foundContractIssue = true;
            REQUIRE(issue["detail"].get<std::string>().find("structured signoff contract") != std::string::npos);
        }
    }
    REQUIRE(foundContractIssue);
}

TEST_CASE("Project audit CLI reports template spec bar-status drift for the selected template",
          "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_template_spec_bar_drift";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-21"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PLANNED"},
                {"requiredSubsystems", json::array({"battle_core"})},
                {"bars", {
                    {"accessibility", "PARTIAL"},
                    {"audio", "PARTIAL"},
                    {"input", "PARTIAL"},
                    {"localization", "PARTIAL"},
                    {"performance", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "docs" / "templates" / "artifact_template_spec.md",
        "# Artifact Template Spec\n\n"
        "Status Date: 2026-04-21\n"
        "Authority: canonical template spec for `artifact_template`\n\n"
        "## Required Subsystems\n\n"
        "| Subsystem | Rationale |\n"
        "| --- | --- |\n"
        "| `battle_core` | Combat support. |\n\n"
        "## Cross-Cutting Minimum Bars\n\n"
        "| Bar | Status | Notes |\n"
        "| --- | --- | --- |\n"
        "| Accessibility | `PLANNED` | Drifted from readiness. |\n"
        "| Audio | `PARTIAL` | Matches readiness. |\n"
        "| Input | `PARTIAL` | Matches readiness. |\n"
        "| Localization | `PARTIAL` | Matches readiness. |\n"
        "| Performance | `PARTIAL` | Matches readiness. |\n");

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["templateSpecArtifactIssueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["issueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["status"] == "parity_mismatch");
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["barsMatch"] == false);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["barMismatches"].is_array());
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["barMismatches"].size() == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["barMismatches"][0]["bar"] ==
            "accessibility");
    REQUIRE(report["issues"].is_array());

    bool foundMismatch = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"] == "template_spec_artifact.bars_mismatch") {
            foundMismatch = true;
            REQUIRE(issue["detail"].get<std::string>().find("Accessibility expected PARTIAL but spec shows PLANNED.") !=
                    std::string::npos);
        }
    }
    REQUIRE(foundMismatch);
}

TEST_CASE("Project audit CLI reports template spec required-subsystem drift for the selected template",
          "[project_audit_cli]") {
    const fs::path tempRoot =
        fs::temp_directory_path() / "urpg_project_audit_cli_template_spec_required_subsystem_drift";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-21"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array({"presentation_runtime", "save_data_core"})},
                {"bars", {
                    {"accessibility", "PARTIAL"},
                    {"audio", "PLANNED"},
                    {"input", "PARTIAL"},
                    {"localization", "PARTIAL"},
                    {"performance", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "docs" / "templates" / "artifact_template_spec.md",
        "# Artifact Template Spec\n\n"
        "Status Date: 2026-04-21\n"
        "Authority: canonical template spec for `artifact_template`\n\n"
        "## Required Subsystems\n\n"
        "| Subsystem | Rationale |\n"
        "| --- | --- |\n"
        "| `presentation_runtime` | Render support. |\n"
        "| `2_5d_mode` | Drifted extra lane. |\n\n"
        "## Cross-Cutting Minimum Bars\n\n"
        "| Bar | Status | Notes |\n"
        "| --- | --- | --- |\n"
        "| Accessibility | `PARTIAL` | Matches readiness. |\n"
        "| Audio | `PLANNED` | Matches readiness. |\n"
        "| Input | `PARTIAL` | Matches readiness. |\n"
        "| Localization | `PARTIAL` | Matches readiness. |\n"
        "| Performance | `PARTIAL` | Matches readiness. |\n");

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["templateSpecArtifactIssueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["issueCount"] == 1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["status"] == "parity_mismatch");
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["requiredSubsystemsMatch"] == false);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["missingRequiredSubsystems"].size() ==
            1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["missingRequiredSubsystems"][0] ==
            "save_data_core");
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["unexpectedRequiredSubsystems"].size() ==
            1);
    REQUIRE(report["governance"]["templateSpecArtifacts"]["expectedArtifacts"][0]["unexpectedRequiredSubsystems"][0] ==
            "2_5d_mode");
    REQUIRE(report["issues"].is_array());

    bool foundMismatch = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"] == "template_spec_artifact.required_subsystems_mismatch") {
            foundMismatch = true;
            REQUIRE(issue["detail"].get<std::string>().find("Missing from spec: [save_data_core].") !=
                    std::string::npos);
            REQUIRE(issue["detail"].get<std::string>().find("Unexpected in spec: [2_5d_mode].") !=
                    std::string::npos);
        }
    }
    REQUIRE(foundMismatch);
}

// ─── S30-T06: fail-closed template-spec coverage for jrpg/visual_novel/turn_based_rpg ──

TEST_CASE("Project audit CLI fails closed on missing spec for jrpg (blocksRelease=true)",
          "[project_audit_cli][s30t06]") {
    const fs::path tempRoot =
        fs::temp_directory_path() / "urpg_project_audit_s30t06_jrpg_missing_spec";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "jrpg"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array({"battle_core", "save_data_core"})},
                {"bars", {{"accessibility", "PARTIAL"}, {"audio", "PARTIAL"}}},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    // Deliberately do NOT create jrpg_spec.md

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "jrpg"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["templateSpecArtifactIssueCount"].get<int>() >= 1);

    bool foundBlockingSpecIssue = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"].get<std::string>().find("template_spec_artifact") != std::string::npos) {
            REQUIRE(issue["blocksRelease"] == true);
            REQUIRE(issue["severity"] == "error");
            foundBlockingSpecIssue = true;
        }
    }
    REQUIRE(foundBlockingSpecIssue);

    fs::remove_all(tempRoot);
}

TEST_CASE("Project audit CLI fails closed on missing spec for visual_novel (blocksRelease=true)",
          "[project_audit_cli][s30t06]") {
    const fs::path tempRoot =
        fs::temp_directory_path() / "urpg_project_audit_s30t06_vn_missing_spec";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "visual_novel"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array({"message_text_core", "save_data_core"})},
                {"bars", {{"accessibility", "PARTIAL"}, {"audio", "PARTIAL"}}},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "visual_novel"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);

    bool foundBlockingSpecIssue = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"].get<std::string>().find("template_spec_artifact") != std::string::npos) {
            REQUIRE(issue["blocksRelease"] == true);
            REQUIRE(issue["severity"] == "error");
            foundBlockingSpecIssue = true;
        }
    }
    REQUIRE(foundBlockingSpecIssue);

    fs::remove_all(tempRoot);
}

TEST_CASE("Project audit CLI fails closed on missing spec for turn_based_rpg (blocksRelease=true)",
          "[project_audit_cli][s30t06]") {
    const fs::path tempRoot =
        fs::temp_directory_path() / "urpg_project_audit_s30t06_tbr_missing_spec";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "turn_based_rpg"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array({"battle_core", "save_data_core", "message_text_core"})},
                {"bars", {{"accessibility", "PARTIAL"}, {"performance", "PARTIAL"}}},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "turn_based_rpg"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);

    bool foundBlockingSpecIssue = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"].get<std::string>().find("template_spec_artifact") != std::string::npos) {
            REQUIRE(issue["blocksRelease"] == true);
            REQUIRE(issue["severity"] == "error");
            foundBlockingSpecIssue = true;
        }
    }
    REQUIRE(foundBlockingSpecIssue);

    fs::remove_all(tempRoot);
}

TEST_CASE("Project audit CLI does NOT fail closed on missing spec for non-candidate templates",
          "[project_audit_cli][s30t06]") {
    // A template that is not in the fail-closed set should still produce warnings, not errors.
    const fs::path tempRoot =
        fs::temp_directory_path() / "urpg_project_audit_s30t06_other_not_fail_closed";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "docs" / "templates");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "sandbox_template"},
                {"status", "EXPERIMENTAL"},
                {"requiredSubsystems", json::array({"battle_core"})},
                {"bars", {{"accessibility", "PLANNED"}}},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "sandbox_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);

    for (const auto& issue : report["issues"]) {
        if (issue["code"].get<std::string>().find("template_spec_artifact") != std::string::npos) {
            // Should be a warning, not an error, and should NOT block release
            REQUIRE(issue["severity"] == "warning");
            REQUIRE(issue["blocksRelease"] == false);
        }
    }

    fs::remove_all(tempRoot);
}
