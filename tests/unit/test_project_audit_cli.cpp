#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
};

fs::path projectAuditExecutablePath() {
#if defined(_WIN32)
    return fs::path(URPG_SOURCE_DIR) / "build" / "dev-ninja-debug" / "urpg_project_audit.exe";
#else
    return fs::path(URPG_SOURCE_DIR) / "build" / "dev-ninja-debug" / "urpg_project_audit";
#endif
}

std::string readTextFile(const fs::path& path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    REQUIRE(stream.good());

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

void writeTextFile(const fs::path& path, const std::string& content) {
    std::ofstream stream(path, std::ios::out | std::ios::binary | std::ios::trunc);
    REQUIRE(stream.good());
    stream << content;
}

void requireGovernanceSectionShape(const json& governance, const std::string& sectionName, bool requireIssueCount = false) {
    if (!governance.contains(sectionName)) {
        return;
    }

    const json& section = governance.at(sectionName);
    REQUIRE(section.is_object());
    if (requireIssueCount) {
        REQUIRE(section.contains("issueCount"));
        REQUIRE(section["issueCount"].is_number_unsigned());
    }

    if (section.contains("path")) {
        REQUIRE(section["path"].is_string());
    }
    if (section.contains("available")) {
        REQUIRE(section["available"].is_boolean());
    }
    if (section.contains("usable")) {
        REQUIRE(section["usable"].is_boolean());
    }
}

bool hasPositiveIssueCount(const json& section) {
    return section.contains("issueCount") && section["issueCount"].is_number_unsigned() && section["issueCount"].get<std::size_t>() > 0;
}

void requireIssueCountConsistencyIfPresent(const json& report, const std::string& topLevelKey, const std::string& sectionKey) {
    if (!report.contains(topLevelKey) || !report.contains("governance") || !report["governance"].is_object()) {
        return;
    }

    REQUIRE(report[topLevelKey].is_number_unsigned());
    const json& governance = report["governance"];
    if (!governance.contains(sectionKey)) {
        return;
    }

    REQUIRE(governance.at(sectionKey).is_object());
    REQUIRE(governance.at(sectionKey).contains("issueCount"));
    REQUIRE(governance.at(sectionKey)["issueCount"].is_number_unsigned());
    REQUIRE(governance.at(sectionKey)["issueCount"] == report[topLevelKey]);
}

ProcessResult runProjectAudit(const std::vector<std::string>& args, const fs::path& workDir) {
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli";
    fs::create_directories(tempRoot);

    const fs::path stdoutPath = tempRoot / "stdout.txt";
    const fs::path stderrPath = tempRoot / "stderr.txt";
    const fs::path exePath = projectAuditExecutablePath();

    REQUIRE(fs::exists(exePath));

    ProcessResult result;

#ifdef _WIN32
    auto toWide = [](const std::string& text) {
        return std::wstring(text.begin(), text.end());
    };

    auto quote = [](const std::wstring& text) {
        return L"\"" + text + L"\"";
    };

    std::wstring commandLine = quote(exePath.wstring());
    for (const auto& arg : args) {
        commandLine += L" ";
        commandLine += quote(toWide(arg));
    }

    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE stdoutHandle = CreateFileW(
        stdoutPath.wstring().c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &securityAttributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    REQUIRE(stdoutHandle != INVALID_HANDLE_VALUE);

    HANDLE stderrHandle = CreateFileW(
        stderrPath.wstring().c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &securityAttributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    REQUIRE(stderrHandle != INVALID_HANDLE_VALUE);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = stdoutHandle;
    startupInfo.hStdError = stderrHandle;

    PROCESS_INFORMATION processInfo{};
    std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back(L'\0');

    const BOOL created = CreateProcessW(
        nullptr,
        mutableCommandLine.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        workDir.wstring().c_str(),
        &startupInfo,
        &processInfo);

    REQUIRE(created == TRUE);

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 1;
    REQUIRE(GetExitCodeProcess(processInfo.hProcess, &exitCode) == TRUE);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    CloseHandle(stdoutHandle);
    CloseHandle(stderrHandle);

    result.exitCode = static_cast<int>(exitCode);
    if (fs::exists(stdoutPath)) {
        result.stdoutText = readTextFile(stdoutPath);
    }
    if (fs::exists(stderrPath)) {
        result.stderrText = readTextFile(stderrPath);
    }
    return result;
#else
    const fs::path previousCwd = fs::current_path();
    fs::current_path(workDir);

    std::ostringstream command;
    command << "\"" << exePath.string() << "\"";
    for (const auto& arg : args) {
        command << " \"" << arg << "\"";
    }
    command << " > \"" << stdoutPath.string() << "\"";
    command << " 2> \"" << stderrPath.string() << "\"";

    result.exitCode = std::system(command.str().c_str());
    fs::current_path(previousCwd);

    if (fs::exists(stdoutPath)) {
        result.stdoutText = readTextFile(stdoutPath);
    }
    if (fs::exists(stderrPath)) {
        result.stderrText = readTextFile(stderrPath);
    }
    return result;
#endif
}

} // namespace

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
    REQUIRE(report["governance"]["performanceArtifacts"].is_object());
    REQUIRE(report["governance"]["releaseSignoffWorkflow"].is_object());
    REQUIRE(report["governance"]["signoffArtifacts"].is_object());
    REQUIRE(report["governance"]["templateSpecArtifacts"].is_object());

    REQUIRE(report["governance"]["assetReport"].contains("available"));
    REQUIRE(report["governance"]["assetReport"].contains("usable"));
    REQUIRE(report["governance"]["assetReport"].contains("issueCount"));
    REQUIRE(report["governance"]["schema"].contains("schemaExists"));
    REQUIRE(report["governance"]["schema"].contains("changelogExists"));
    REQUIRE(report["governance"]["schema"].contains("mentionsSchemaVersion"));
    REQUIRE(report["governance"]["projectSchema"].contains("available"));
    REQUIRE(report["governance"]["localizationArtifacts"].contains("issueCount"));
    REQUIRE(report["governance"]["localizationEvidence"].contains("issueCount"));
    REQUIRE(report["governance"]["exportArtifacts"].contains("issueCount"));

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
    requireGovernanceSectionShape(report["governance"], "performanceArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "releaseSignoffWorkflow", true);
    requireGovernanceSectionShape(report["governance"], "signoffArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "templateSpecArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");
    requireIssueCountConsistencyIfPresent(report, "localizationEvidenceIssueCount", "localizationEvidence");
    requireIssueCountConsistencyIfPresent(report, "accessibilityArtifactIssueCount", "accessibilityArtifacts");
    requireIssueCountConsistencyIfPresent(report, "audioArtifactIssueCount", "audioArtifacts");
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
    requireGovernanceSectionShape(report["governance"], "performanceArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "releaseSignoffWorkflow", true);
    requireGovernanceSectionShape(report["governance"], "signoffArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "templateSpecArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");
    requireIssueCountConsistencyIfPresent(report, "localizationEvidenceIssueCount", "localizationEvidence");
    requireIssueCountConsistencyIfPresent(report, "accessibilityArtifactIssueCount", "accessibilityArtifacts");
    requireIssueCountConsistencyIfPresent(report, "audioArtifactIssueCount", "audioArtifacts");
    requireIssueCountConsistencyIfPresent(report, "performanceArtifactIssueCount", "performanceArtifacts");
    requireIssueCountConsistencyIfPresent(report, "releaseSignoffWorkflowIssueCount", "releaseSignoffWorkflow");
    requireIssueCountConsistencyIfPresent(report, "signoffArtifactIssueCount", "signoffArtifacts");
    requireIssueCountConsistencyIfPresent(report, "templateSpecArtifactIssueCount", "templateSpecArtifacts");

    CHECK(report["summary"].get<std::string>().find("Selected template artifact-template is PARTIAL.") != std::string::npos);
    CHECK(report["issues"].is_array());
    CHECK(report["assetGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
    CHECK(report["schemaGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
}

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
    REQUIRE(battleArtifact["signoffContract"]["promotionRequiresHumanReview"] == true);
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
