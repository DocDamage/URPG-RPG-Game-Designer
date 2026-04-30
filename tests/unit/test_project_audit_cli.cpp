#include "project_audit_cli_test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace urpg::tests::project_audit_cli;

TEST_CASE("Project audit CLI emits parseable JSON from the default repo data", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path readinessPath = repoRoot / "content" / "readiness" / "readiness_status.json";

    const ProcessResult result = runProjectAudit({"--json", "--input", readinessPath.string()}, repoRoot);

    REQUIRE(result.exitCode == 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const json report = json::parse(result.stdoutText);
    REQUIRE(report.contains("schemaVersion"));
    REQUIRE(report.contains("headline"));
    REQUIRE(report.contains("summary"));
    REQUIRE(report.contains("releaseBlockerCount"));
    REQUIRE(report.contains("exportBlockerCount"));
    REQUIRE(report.contains("templateContext"));
    REQUIRE(report.contains("governance"));
    REQUIRE(report.contains("assetGovernanceIssueCount"));
    REQUIRE(report.contains("schemaGovernanceIssueCount"));
    REQUIRE(report.contains("issues"));

    REQUIRE(report["schemaVersion"].is_string());
    REQUIRE(report["headline"].is_string());
    REQUIRE(report["summary"].is_string());
    REQUIRE(report["releaseBlockerCount"].is_number_unsigned());
    REQUIRE(report["exportBlockerCount"].is_number_unsigned());
    REQUIRE(report["templateContext"].is_object());
    REQUIRE(report["templateContext"]["id"].is_string());
    REQUIRE(report["templateContext"]["status"].is_string());
    REQUIRE(report["governance"].is_object());
    REQUIRE(report["assetGovernanceIssueCount"].is_number_unsigned());
    REQUIRE(report["schemaGovernanceIssueCount"].is_number_unsigned());
    REQUIRE(report["issues"].is_array());

    REQUIRE(report["governance"].contains("assetReport"));
    REQUIRE(report["governance"].contains("schema"));
    REQUIRE(report["governance"].contains("projectSchema"));
    REQUIRE(report["governance"].contains("localizationArtifacts"));
    REQUIRE(report["governance"].contains("localizationEvidence"));
    REQUIRE(report["governance"].contains("exportArtifacts"));
    REQUIRE(report["governance"].contains("accessibilityArtifacts"));
    REQUIRE(report["governance"].contains("audioArtifacts"));
    REQUIRE(report["governance"].contains("characterArtifacts"));
    REQUIRE(report["governance"].contains("modArtifacts"));
    REQUIRE(report["governance"].contains("analyticsArtifacts"));
    REQUIRE(report["governance"].contains("performanceArtifacts"));
    REQUIRE(report["governance"].contains("releaseSignoffWorkflow"));
    REQUIRE(report["governance"].contains("signoffArtifacts"));
    REQUIRE(report["governance"].contains("templateSpecArtifacts"));
    REQUIRE(report["governance"]["assetReport"].is_object());
    REQUIRE(report["governance"]["schema"].is_object());
    REQUIRE(report["governance"]["projectSchema"].is_object());
    REQUIRE(report["governance"]["localizationArtifacts"].is_object());
    REQUIRE(report["governance"]["localizationEvidence"].is_object());
    REQUIRE(report["governance"]["exportArtifacts"].is_object());
    REQUIRE(report["governance"]["accessibilityArtifacts"].is_object());
    REQUIRE(report["governance"]["audioArtifacts"].is_object());
    REQUIRE(report["governance"]["characterArtifacts"].is_object());
    REQUIRE(report["governance"]["modArtifacts"].is_object());
    REQUIRE(report["governance"]["analyticsArtifacts"].is_object());
    REQUIRE(report["governance"]["performanceArtifacts"].is_object());
    REQUIRE(report["governance"]["releaseSignoffWorkflow"].is_object());
    REQUIRE(report["governance"]["signoffArtifacts"].is_object());
    REQUIRE(report["governance"]["templateSpecArtifacts"].is_object());

    REQUIRE(report["governance"]["assetReport"].contains("available"));
    REQUIRE(report["governance"]["assetReport"].contains("usable"));
    REQUIRE(report["governance"]["assetReport"].contains("issueCount"));
    REQUIRE(report["governance"]["assetReport"].contains("normalizedCount"));
    REQUIRE(report["governance"]["assetReport"].contains("promotedCount"));
    REQUIRE(report["governance"]["assetReport"].contains("promotedVisualLaneCount"));
    REQUIRE(report["governance"]["assetReport"].contains("promotedAudioLaneCount"));
    REQUIRE(report["governance"]["assetReport"].contains("wysiwygSmokeProofCount"));
    REQUIRE(report["governance"]["schema"].contains("schemaExists"));
    REQUIRE(report["governance"]["schema"].contains("changelogExists"));
    REQUIRE(report["governance"]["schema"].contains("mentionsSchemaVersion"));
    REQUIRE(report["governance"]["projectSchema"].contains("available"));
    REQUIRE(report["governance"]["localizationArtifacts"].contains("issueCount"));
    REQUIRE(report["governance"]["localizationEvidence"].contains("issueCount"));
    REQUIRE(report["governance"]["exportArtifacts"].contains("issueCount"));
    REQUIRE(report["governance"]["exportArtifacts"]["issueCount"] == 0);
    REQUIRE_FALSE(reportContainsIssueCode(report, "export_artifact.cli_missing"));
    REQUIRE(report["governance"]["assetReport"]["issueCount"] == 0);
    REQUIRE(report["governance"]["assetReport"]["normalizedCount"].get<std::int64_t>() > 0);
    REQUIRE(report["governance"]["assetReport"]["promotedCount"].get<std::int64_t>() > 0);
    REQUIRE(report["governance"]["assetReport"]["promotedVisualLaneCount"].get<std::int64_t>() > 0);
    REQUIRE(report["governance"]["assetReport"]["promotedAudioLaneCount"].get<std::int64_t>() > 0);
    REQUIRE(report["governance"]["assetReport"]["wysiwygSmokeProofCount"].get<std::int64_t>() > 0);

    REQUIRE(report["governance"]["assetReport"]["issueCount"] == report["assetGovernanceIssueCount"]);
    CHECK(report["schemaGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
    CHECK(report["assetGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());

    requireGovernanceSectionShape(report["governance"], "assetReport", true);
    requireGovernanceSectionShape(report["governance"], "schema", false);
    requireGovernanceSectionShape(report["governance"], "projectSchema", false);
    requireGovernanceSectionShape(report["governance"], "localizationArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "localizationEvidence", true);
    requireGovernanceSectionShape(report["governance"], "exportArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "inputArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "accessibilityArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "audioArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "characterArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "modArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "analyticsArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "performanceArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "releaseSignoffWorkflow", true);
    requireGovernanceSectionShape(report["governance"], "signoffArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "templateSpecArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");
    requireIssueCountConsistencyIfPresent(report, "localizationEvidenceIssueCount", "localizationEvidence");
    requireIssueCountConsistencyIfPresent(report, "accessibilityArtifactIssueCount", "accessibilityArtifacts");
    requireIssueCountConsistencyIfPresent(report, "audioArtifactIssueCount", "audioArtifacts");
    requireIssueCountConsistencyIfPresent(report, "characterArtifactIssueCount", "characterArtifacts");
    requireIssueCountConsistencyIfPresent(report, "modArtifactIssueCount", "modArtifacts");
    requireIssueCountConsistencyIfPresent(report, "analyticsArtifactIssueCount", "analyticsArtifacts");
    requireIssueCountConsistencyIfPresent(report, "performanceArtifactIssueCount", "performanceArtifacts");
    requireIssueCountConsistencyIfPresent(report, "releaseSignoffWorkflowIssueCount", "releaseSignoffWorkflow");
    requireIssueCountConsistencyIfPresent(report, "signoffArtifactIssueCount", "signoffArtifacts");
    requireIssueCountConsistencyIfPresent(report, "templateSpecArtifactIssueCount", "templateSpecArtifacts");

    if (report["assetGovernanceIssueCount"].get<std::size_t>() > 0 ||
        report["schemaGovernanceIssueCount"].get<std::size_t>() > 0 ||
        hasPositiveIssueCount(report["governance"]["localizationArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["localizationEvidence"]) ||
        hasPositiveIssueCount(report["governance"]["exportArtifacts"]) ||
        (report["governance"].contains("inputArtifacts") && hasPositiveIssueCount(report["governance"]["inputArtifacts"])) ||
        hasPositiveIssueCount(report["governance"]["accessibilityArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["audioArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["characterArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["modArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["analyticsArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["performanceArtifacts"])) {
        CHECK(report["summary"].get<std::string>().find("Asset governance issues:") != std::string::npos);
        CHECK(report["summary"].get<std::string>().find("Schema governance issues:") != std::string::npos);
        if (hasPositiveIssueCount(report["governance"]["localizationArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Localization artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["localizationEvidence"])) {
            CHECK(report["summary"].get<std::string>().find("Localization evidence issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["exportArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Export artifact issues:") != std::string::npos);
        }
        if (report["governance"].contains("inputArtifacts") && hasPositiveIssueCount(report["governance"]["inputArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Input artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["accessibilityArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Accessibility artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["audioArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Audio artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["characterArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Character artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["modArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Mod artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["analyticsArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Analytics artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["performanceArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Performance artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["releaseSignoffWorkflow"])) {
            CHECK(report["summary"].get<std::string>().find("Release-signoff workflow issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["signoffArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Signoff artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["templateSpecArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Template-spec artifact issues:") != std::string::npos);
        }
    }
}

TEST_CASE("Project audit CLI flags normalized or promoted asset intake regressions", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_asset_regression";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot);
    const fs::path readinessPath = repoRoot / "content" / "readiness" / "readiness_status.json";

    const auto runWithAssetSummary = [&](const json& summary) {
        const fs::path reportPath = tempRoot / ("asset_report_" + std::to_string(summary.value("normalized", 0)) +
                                                "_" + std::to_string(summary.value("promoted", 0)) + ".json");
        writeTextFile(reportPath, json{{"summary", summary}, {"sources", json::array()}}.dump(2));
        const ProcessResult result = runProjectAudit(
            {"--json", "--input", readinessPath.string(), "--asset-report", reportPath.string()},
            repoRoot);
        REQUIRE(result.exitCode == 0);
        return json::parse(result.stdoutText);
    };

    const json missingPromoted = runWithAssetSummary({
        {"normalized", 1},
        {"promoted", 0},
        {"promoted_visual_lanes", 1},
        {"promoted_audio_lanes", 1},
        {"wysiwyg_smoke_proofs", 1},
    });
    REQUIRE(missingPromoted["governance"]["assetReport"]["issueCount"].get<std::size_t>() > 0);
    REQUIRE(reportContainsIssueCode(missingPromoted, "asset_report.no_promoted_intake"));

    const json missingNormalized = runWithAssetSummary({
        {"normalized", 0},
        {"promoted", 1},
        {"promoted_visual_lanes", 1},
        {"promoted_audio_lanes", 1},
        {"wysiwyg_smoke_proofs", 1},
    });
    REQUIRE(missingNormalized["governance"]["assetReport"]["issueCount"].get<std::size_t>() > 0);
    REQUIRE(reportContainsIssueCode(missingNormalized, "asset_report.no_normalized_intake"));

    const json missingVisualAudioProof = runWithAssetSummary({
        {"normalized", 1},
        {"promoted", 1},
        {"promoted_visual_lanes", 0},
        {"promoted_audio_lanes", 0},
        {"wysiwyg_smoke_proofs", 0},
    });
    REQUIRE(reportContainsIssueCode(missingVisualAudioProof, "asset_report.no_promoted_visual_lane"));
    REQUIRE(reportContainsIssueCode(missingVisualAudioProof, "asset_report.no_promoted_audio_lane"));
    REQUIRE(reportContainsIssueCode(missingVisualAudioProof, "asset_report.no_wysiwyg_smoke_proof"));

    fs::remove_all(tempRoot);
}

TEST_CASE("Project audit CLI selects a requested template from a synthetic readiness payload", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli";
    fs::create_directories(tempRoot);
    fs::create_directories(tempRoot / "docs" / "templates");

    const fs::path inputPath = tempRoot / "synthetic_readiness.json";
    writeTextFile(inputPath, json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-20"},
        {"templates", json::array({
            {
                {"id", "custom-template"},
                {"status", "READY"},
                {"requiredSubsystems", json::array({"core"})}
            }
        })},
        {"subsystems", json::array({
            {
                {"id", "core"},
                {"status", "READY"},
                {"summary", "Core systems are ready."}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "docs" / "templates" / "custom-template_spec.md",
        "# Custom Template Spec\n\n"
        "Status Date: 2026-04-20\n"
        "Authority: canonical template spec for `custom-template`\n\n"
        "## Required Subsystems\n\n"
        "| Subsystem | Rationale |\n"
        "| --- | --- |\n"
        "| `core` | Core systems are ready. |\n");

    const ProcessResult result =
        runProjectAudit({"--json", "--input", inputPath.string(), "--template", "custom-template"}, tempRoot);

    REQUIRE(result.exitCode == 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const json report = json::parse(result.stdoutText);
    REQUIRE(report["templateContext"]["id"] == "custom-template");
    REQUIRE(report["templateContext"]["status"] == "READY");
    REQUIRE(report["issues"].is_array());
    REQUIRE(report["releaseBlockerCount"] == 0);
    REQUIRE(report["exportBlockerCount"] == 0);
}

TEST_CASE("Project audit CLI keeps governance shape stable for conservative artifact checks",
          "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_governance";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-20"},
        {"subsystems", json::array({
            {
                {"id", "core"},
                {"status", "READY"},
                {"summary", "Core systems are ready."},
                {"mainGaps", json::array()},
                {"evidence", {
                    {"runtimeOwner", true},
                    {"editorSurface", true},
                    {"schemaMigration", true},
                    {"diagnostics", true},
                    {"testsValidation", true},
                    {"docsAligned", true}
                }}
            }
        })},
        {"templates", json::array({
            {
                {"id", "artifact-template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array({"core"})},
                {"bars", {
                    {"accessibility", "PLANNED"},
                    {"audio", "PARTIAL"},
                    {"input", "PARTIAL"},
                    {"localization", "PLANNED"},
                    {"performance", "PARTIAL"}
                }},
                {"safeScope", "Synthetic template for artifact checks."},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact-template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["assetGovernanceIssueCount"].is_number_unsigned());
    REQUIRE(report["schemaGovernanceIssueCount"].is_number_unsigned());
    REQUIRE(report["governance"].is_object());
    REQUIRE(report["governance"].contains("assetReport"));
    REQUIRE(report["governance"].contains("schema"));
    REQUIRE(report["governance"].contains("projectSchema"));
    REQUIRE(report["governance"].contains("localizationArtifacts"));
    REQUIRE(report["governance"].contains("localizationEvidence"));
    REQUIRE(report["governance"].contains("exportArtifacts"));
    REQUIRE(report["governance"].contains("accessibilityArtifacts"));
    REQUIRE(report["governance"].contains("audioArtifacts"));
    REQUIRE(report["governance"].contains("characterArtifacts"));
    REQUIRE(report["governance"].contains("modArtifacts"));
    REQUIRE(report["governance"].contains("analyticsArtifacts"));
    REQUIRE(report["governance"].contains("performanceArtifacts"));
    REQUIRE(report["governance"].contains("releaseSignoffWorkflow"));
    REQUIRE(report["governance"].contains("signoffArtifacts"));
    REQUIRE(report["governance"].contains("templateSpecArtifacts"));

    requireGovernanceSectionShape(report["governance"], "assetReport", true);
    requireGovernanceSectionShape(report["governance"], "schema", false);
    requireGovernanceSectionShape(report["governance"], "projectSchema", false);
    requireGovernanceSectionShape(report["governance"], "localizationArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "localizationEvidence", true);
    requireGovernanceSectionShape(report["governance"], "exportArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "inputArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "accessibilityArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "audioArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "characterArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "modArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "analyticsArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "performanceArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "releaseSignoffWorkflow", true);
    requireGovernanceSectionShape(report["governance"], "signoffArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "templateSpecArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");
    requireIssueCountConsistencyIfPresent(report, "localizationEvidenceIssueCount", "localizationEvidence");
    requireIssueCountConsistencyIfPresent(report, "accessibilityArtifactIssueCount", "accessibilityArtifacts");
    requireIssueCountConsistencyIfPresent(report, "audioArtifactIssueCount", "audioArtifacts");
    requireIssueCountConsistencyIfPresent(report, "characterArtifactIssueCount", "characterArtifacts");
    requireIssueCountConsistencyIfPresent(report, "modArtifactIssueCount", "modArtifacts");
    requireIssueCountConsistencyIfPresent(report, "analyticsArtifactIssueCount", "analyticsArtifacts");
    requireIssueCountConsistencyIfPresent(report, "performanceArtifactIssueCount", "performanceArtifacts");
    requireIssueCountConsistencyIfPresent(report, "releaseSignoffWorkflowIssueCount", "releaseSignoffWorkflow");
    requireIssueCountConsistencyIfPresent(report, "signoffArtifactIssueCount", "signoffArtifacts");
    requireIssueCountConsistencyIfPresent(report, "templateSpecArtifactIssueCount", "templateSpecArtifacts");

    CHECK(report["summary"].get<std::string>().find("Selected template artifact-template is PARTIAL.") != std::string::npos);
    CHECK(report["issues"].is_array());
    CHECK(report["assetGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
    CHECK(report["schemaGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
}

TEST_CASE("Project schema defines canonical governance sections and fixture shape", "[project_audit_cli][project_schema]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const json schema = json::parse(readTextFile(repoRoot / "content" / "schemas" / "project.schema.json"));
    const json fixture = json::parse(readTextFile(repoRoot / "content" / "fixtures" / "project_governance_fixture.json"));

    REQUIRE(schema["properties"].contains("localization"));
    REQUIRE(schema["properties"]["localization"]["type"] == "object");
    REQUIRE(jsonArrayContainsString(schema["properties"]["localization"]["required"], "bundleRoot"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["localization"]["required"], "schemaPath"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["localization"]["required"], "reportPath"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["localization"]["required"], "requiredLocales"));
    REQUIRE(schema["properties"]["localization"]["properties"]["schemaPath"]["const"] ==
            "content/schemas/localization_bundle.schema.json");
    REQUIRE(schema["properties"]["localization"]["properties"]["reportPath"]["const"] ==
            "imports/reports/localization/localization_consistency_report.json");

    REQUIRE(schema["properties"].contains("input"));
    REQUIRE(schema["properties"]["input"]["type"] == "object");
    REQUIRE(jsonArrayContainsString(schema["properties"]["input"]["required"], "bindingSchemaPath"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["input"]["required"], "controllerBindingSchemaPath"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["input"]["required"], "fixturePath"));

    REQUIRE(schema["properties"].contains("exportProfiles"));
    REQUIRE(schema["properties"]["exportProfiles"]["type"] == "array");
    REQUIRE(jsonArrayContainsString(schema["properties"]["exportProfiles"]["items"]["required"], "id"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["exportProfiles"]["items"]["required"], "target"));
    REQUIRE(jsonArrayContainsString(schema["properties"]["exportProfiles"]["items"]["required"], "configSchemaPath"));
    REQUIRE(jsonArrayContainsString(
        schema["properties"]["exportProfiles"]["items"]["required"],
        "validationReportSchemaPath"));

    REQUIRE(fixture["localization"]["schemaPath"] == "content/schemas/localization_bundle.schema.json");
    REQUIRE(fixture["localization"]["reportPath"] ==
            "imports/reports/localization/localization_consistency_report.json");
    REQUIRE(fixture["input"]["bindingSchemaPath"] == "content/schemas/input_bindings.schema.json");
    REQUIRE(fixture["input"]["controllerBindingSchemaPath"] == "content/schemas/controller_bindings.schema.json");
    REQUIRE(fixture["exportProfiles"].is_array());
    REQUIRE(fixture["exportProfiles"][0]["configSchemaPath"] == "content/schemas/export_config.schema.json");
}

TEST_CASE("Project audit accepts the canonical project governance schema vocabulary", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_project_schema_positive";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "content" / "schemas");

    writeProjectGovernanceReadinessFixture(tempRoot / "content" / "readiness" / "synthetic_readiness.json");
    writeTextFile(
        tempRoot / "content" / "schemas" / "project.schema.json",
        readTextFile(repoRoot / "content" / "schemas" / "project.schema.json"));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "governance_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["projectArtifactIssueCount"] == 0);
    REQUIRE(report["governance"]["projectSchema"]["canonicalLocalizationProperty"] == "localization");
    REQUIRE(report["governance"]["projectSchema"]["canonicalInputProperty"] == "input");
    REQUIRE(report["governance"]["projectSchema"]["canonicalExportProperty"] == "exportProfiles");
    REQUIRE(report["governance"]["projectSchema"]["hasLocalizationSection"] == true);
    REQUIRE(report["governance"]["projectSchema"]["hasInputSection"] == true);
    REQUIRE(report["governance"]["projectSchema"]["hasExportSection"] == true);
    REQUIRE_FALSE(reportContainsIssueCode(report, "project_schema.localization_missing"));
    REQUIRE_FALSE(reportContainsIssueCode(report, "project_schema.input_governance_missing"));
    REQUIRE_FALSE(reportContainsIssueCode(report, "project_schema.export_governance_missing"));
}

TEST_CASE("Project audit reports missing canonical project governance sections", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_project_schema_missing";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "content" / "schemas");

    writeProjectGovernanceReadinessFixture(tempRoot / "content" / "readiness" / "synthetic_readiness.json");
    writeTextFile(
        tempRoot / "content" / "schemas" / "project.schema.json",
        json{
            {"$schema", "https://json-schema.org/draft/2020-12/schema"},
            {"type", "object"},
            {"properties", {{"determinism", {{"type", "object"}}}}}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "governance_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["projectArtifactIssueCount"] == 3);
    REQUIRE(report["governance"]["projectSchema"]["hasLocalizationSection"] == false);
    REQUIRE(report["governance"]["projectSchema"]["hasInputSection"] == false);
    REQUIRE(report["governance"]["projectSchema"]["hasExportSection"] == false);
    REQUIRE(reportContainsIssueCode(report, "project_schema.localization_missing"));
    REQUIRE(reportContainsIssueCode(report, "project_schema.input_governance_missing"));
    REQUIRE(reportContainsIssueCode(report, "project_schema.export_governance_missing"));
}

TEST_CASE("Project audit reports malformed canonical project governance sections", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_project_schema_malformed";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "content" / "schemas");

    writeProjectGovernanceReadinessFixture(tempRoot / "content" / "readiness" / "synthetic_readiness.json");
    writeTextFile(
        tempRoot / "content" / "schemas" / "project.schema.json",
        json{
            {"$schema", "https://json-schema.org/draft/2020-12/schema"},
            {"type", "object"},
            {"properties", {
                {"localization", {{"type", "string"}}},
                {"input", {
                    {"type", "object"},
                    {"required", json::array({"fixturePath"})},
                    {"properties", {{"fixturePath", {{"type", "string"}}}}}
                }},
                {"exportProfiles", {{"type", "object"}}}
            }}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "governance_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["projectArtifactIssueCount"] == 3);
    REQUIRE(report["governance"]["projectSchema"]["hasLocalizationSection"] == false);
    REQUIRE(report["governance"]["projectSchema"]["hasInputSection"] == false);
    REQUIRE(report["governance"]["projectSchema"]["hasExportSection"] == false);
    REQUIRE(reportContainsIssueCode(report, "project_schema.localization_malformed"));
    REQUIRE(reportContainsIssueCode(report, "project_schema.input_governance_malformed"));
    REQUIRE(reportContainsIssueCode(report, "project_schema.export_governance_malformed"));
}
