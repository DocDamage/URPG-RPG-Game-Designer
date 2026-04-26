#include <catch2/catch_test_macros.hpp>

#include "engine/core/project/template_certification.h"
#include "tools/audit/project_completeness_score.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace {

std::string readText(const std::filesystem::path& path) {
    std::ifstream in(path);
    REQUIRE(in.is_open());
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::filesystem::path projectAuditExecutable() {
#if defined(_WIN32)
    return std::filesystem::path(URPG_SOURCE_DIR) / "build" / "dev-ninja-debug" / "urpg_project_audit.exe";
#else
    return std::filesystem::path(URPG_SOURCE_DIR) / "build" / "dev-ninja-debug" / "urpg_project_audit";
#endif
}

int runPowerShellScript(const std::filesystem::path& script, const std::string& args = "") {
    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + script.string() + "\" " + args;
    return std::system(command.c_str());
}

} // namespace

TEST_CASE("Template certification covers all conservative template suites", "[certification]") {
    urpg::project::TemplateCertification certification;
    const auto suites = certification.defaultSuites();

    REQUIRE(suites.size() == 9);
    REQUIRE(suites[0].templateId == "jrpg");
    REQUIRE(suites[1].templateId == "visual_novel");
    REQUIRE(suites[2].templateId == "turn_based_rpg");
    REQUIRE(suites[3].templateId == "tactics_rpg");
    REQUIRE(suites[4].templateId == "arpg");
    REQUIRE(suites[5].templateId == "monster_collector_rpg");
    REQUIRE(suites[6].templateId == "cozy_life_rpg");
    REQUIRE(suites[7].templateId == "metroidvania_lite");
    REQUIRE(suites[8].templateId == "2_5d_rpg");
}

TEST_CASE("Template certification fails when required template loop is missing", "[certification]") {
    const auto fixturePath = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" /
                             "template_certification" / "negative_missing_loop.json";
    const auto project = nlohmann::json::parse(readText(fixturePath));

    urpg::project::TemplateCertification certification;
    const auto report = certification.certify(project, project.value("templateId", ""));

    REQUIRE_FALSE(report.passed);
    REQUIRE(report.issues.size() == 1);
    REQUIRE(report.issues[0].code == "missing_required_loop");
    REQUIRE(report.toJson()["schema"] == "urpg.template_certification.v1");
}

TEST_CASE("Template certification positive fixture passes", "[certification]") {
    const auto fixturePath = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" /
                             "template_certification" / "positive_jrpg.json";
    const auto project = nlohmann::json::parse(readText(fixturePath));

    urpg::project::TemplateCertification certification;
    const auto report = certification.certify(project, project.value("templateId", ""));

    REQUIRE(report.passed);
    REQUIRE(report.issues.empty());
}

TEST_CASE("Completeness scoring ignores disabled optional features", "[project_audit][certification]") {
    const nlohmann::json project = {
        {"implementedFeatures", nlohmann::json::array({"battle_loop"})},
        {"disabledOptionalFeatures", nlohmann::json::array({"fishing_loop"})},
    };

    const auto score = urpg::tools::audit::scoreProjectCompleteness(project, {
                                                                                 {"battle_loop", false},
                                                                                 {"fishing_loop", true},
                                                                             });

    REQUIRE(score.applicable == 1);
    REQUIRE(score.satisfied == 1);
    REQUIRE(score.score == 1.0);
    REQUIRE(score.toJson()["advisoryOnly"] == true);
}

TEST_CASE("Project audit exposes completeness as non-authoritative advisory", "[project_audit][certification]") {
    const auto exe = projectAuditExecutable();
    REQUIRE(std::filesystem::exists(exe));

    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_project_audit_ffs17.json";
    const auto readinessPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "content" / "readiness" / "readiness_status.json";
    const std::string command = "powershell -ExecutionPolicy Bypass -Command \"& '" + exe.string() +
                                "' --json --input '" + readinessPath.string() + "' | Out-File -Encoding utf8 '" +
                                outputPath.string() + "'\"";
    REQUIRE(std::system(command.c_str()) == 0);

    const auto report = nlohmann::json::parse(readText(outputPath));
    REQUIRE(report.contains("completenessAdvisory"));
    REQUIRE(report["completenessAdvisory"]["advisoryOnly"] == true);
    REQUIRE(report["completenessAdvisory"]["nonAuthoritative"] == true);
}

TEST_CASE("FFS-17 governance scripts accept positive fixtures and reject negative fixtures",
          "[certification][project_audit]") {
    const auto repoRoot = std::filesystem::path(URPG_SOURCE_DIR);

    REQUIRE(runPowerShellScript(repoRoot / "tools" / "ci" / "check_template_certification.ps1") == 0);
    REQUIRE(runPowerShellScript(repoRoot / "tools" / "ci" / "check_feature_governance.ps1") == 0);
    REQUIRE(runPowerShellScript(repoRoot / "tools" / "docs" / "check_future_feature_docs.ps1") == 0);

    const auto negativeRoot = repoRoot / "content" / "fixtures" / "feature_governance" / "negative";
    REQUIRE(runPowerShellScript(repoRoot / "tools" / "ci" / "check_feature_governance.ps1",
                                "-FixtureRoot \"" + negativeRoot.string() + "\" -ExpectFailure") == 0);
}
