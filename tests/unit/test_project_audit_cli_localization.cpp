#include "project_audit_cli_test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace urpg::tests::project_audit_cli;

TEST_CASE("Project audit CLI surfaces localization consistency evidence drift", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_localization_evidence";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "imports" / "reports" / "localization");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-21"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", {
                    {"localization", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "imports" / "reports" / "localization" / "localization_consistency_report.json",
        json{
            {"schemaVersion", "1.0.0"},
            {"status", "missing_keys"},
            {"summary", {
                {"hasBundles", true},
                {"bundleCount", 2},
                {"masterLocale", "en"},
                {"missingLocaleCount", 1},
                {"missingKeyCount", 2},
                {"extraKeyCount", 1}
            }},
            {"bundles", json::array({
                {
                    {"path", "content/localization/en.json"},
                    {"locale", "en"},
                    {"keyCount", 3},
                    {"missingKeys", json::array()},
                    {"extraKeys", json::array()}
                },
                {
                    {"path", "content/localization/fr.json"},
                    {"locale", "fr"},
                    {"keyCount", 2},
                    {"missingKeys", json::array({"ui.quit", "battle.attack"})},
                    {"extraKeys", json::array({"ui.legacy"})}
                }
            })}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["localizationEvidenceIssueCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["issueCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["usable"] == true);
    REQUIRE(report["governance"]["localizationEvidence"]["status"] == "missing_keys");
    REQUIRE(report["governance"]["localizationEvidence"]["missingKeyCount"] == 2);
    REQUIRE(report["governance"]["localizationEvidence"]["missingLocaleCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["extraKeyCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["bundles"].is_array());
    REQUIRE(report["summary"].get<std::string>().find("Localization evidence issues: 1.") != std::string::npos);

    bool foundLocalizationEvidenceIssue = false;
    for (const auto& issue : report["issues"]) {
        if (issue["code"] == "localization_report.missing_keys") {
            foundLocalizationEvidenceIssue = true;
            REQUIRE(issue["detail"].get<std::string>().find("missing keys") != std::string::npos);
        }
    }
    REQUIRE(foundLocalizationEvidenceIssue);
}

TEST_CASE("Project audit CLI keeps clean localization consistency evidence usable", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_localization_evidence_clean";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "imports" / "reports" / "localization");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-21"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", {
                    {"localization", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "imports" / "reports" / "localization" / "localization_consistency_report.json",
        json{
            {"schemaVersion", "1.0.0"},
            {"status", "passed"},
            {"summary", {
                {"hasBundles", true},
                {"bundleCount", 2},
                {"masterLocale", "en"},
                {"missingLocaleCount", 0},
                {"missingKeyCount", 0},
                {"extraKeyCount", 0}
            }},
            {"bundles", json::array()}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["localizationEvidenceIssueCount"] == 0);
    REQUIRE(report["governance"]["localizationEvidence"]["issueCount"] == 0);
    REQUIRE(report["governance"]["localizationEvidence"]["usable"] == true);
    REQUIRE(report["governance"]["localizationEvidence"]["status"] == "passed");
    REQUIRE(report["governance"]["localizationEvidence"]["missingKeyCount"] == 0);
}

TEST_CASE("Project audit CLI keeps no-bundle localization evidence usable", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_localization_evidence_no_bundles";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "imports" / "reports" / "localization");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", {
                    {"localization", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "imports" / "reports" / "localization" / "localization_consistency_report.json",
        json{
            {"schemaVersion", "1.0.0"},
            {"status", "no_bundles"},
            {"summary", {
                {"hasBundles", false},
                {"bundleCount", 0},
                {"masterLocale", nullptr},
                {"missingLocaleCount", 0},
                {"missingKeyCount", 0},
                {"extraKeyCount", 0}
            }},
            {"bundles", json::array()}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["localizationEvidenceIssueCount"] == 0);
    REQUIRE(report["governance"]["localizationEvidence"]["issueCount"] == 0);
    REQUIRE(report["governance"]["localizationEvidence"]["usable"] == true);
    REQUIRE(report["governance"]["localizationEvidence"]["status"] == "no_bundles");
    REQUIRE(report["governance"]["localizationEvidence"]["hasBundles"] == false);
    REQUIRE(report["governance"]["localizationEvidence"]["bundleCount"] == 0);
}

TEST_CASE("Project audit CLI reports malformed localization evidence without explicit path", "[project_audit_cli]") {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli_localization_evidence_malformed";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot / "content" / "readiness");
    fs::create_directories(tempRoot / "imports" / "reports" / "localization");

    writeTextFile(tempRoot / "content" / "readiness" / "synthetic_readiness.json", json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "artifact_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", {
                    {"localization", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));

    writeTextFile(
        tempRoot / "imports" / "reports" / "localization" / "localization_consistency_report.json",
        json{
            {"schemaVersion", "1.0.0"},
            {"status", "failed"},
            {"summary", {
                {"hasBundles", true},
                {"bundleCount", 1}
            }}
        }.dump(2));

    const ProcessResult result = runProjectAudit(
        {"--json", "--input", (tempRoot / "content" / "readiness" / "synthetic_readiness.json").string(),
         "--template", "artifact_template"},
        tempRoot);

    REQUIRE(result.exitCode == 0);
    const json report = json::parse(result.stdoutText);
    REQUIRE(report["localizationEvidenceIssueCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["issueCount"] == 1);
    REQUIRE(report["governance"]["localizationEvidence"]["usable"] == false);
    REQUIRE(reportContainsIssueCode(report, "localization_report.summary.counters_missing"));
}
