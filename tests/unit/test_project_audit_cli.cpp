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
    REQUIRE(report["governance"].contains("exportArtifacts"));
    REQUIRE(report["governance"]["assetReport"].is_object());
    REQUIRE(report["governance"]["schema"].is_object());
    REQUIRE(report["governance"]["projectSchema"].is_object());
    REQUIRE(report["governance"]["localizationArtifacts"].is_object());
    REQUIRE(report["governance"]["exportArtifacts"].is_object());

    REQUIRE(report["governance"]["assetReport"].contains("available"));
    REQUIRE(report["governance"]["assetReport"].contains("usable"));
    REQUIRE(report["governance"]["assetReport"].contains("issueCount"));
    REQUIRE(report["governance"]["schema"].contains("schemaExists"));
    REQUIRE(report["governance"]["schema"].contains("changelogExists"));
    REQUIRE(report["governance"]["schema"].contains("mentionsSchemaVersion"));
    REQUIRE(report["governance"]["projectSchema"].contains("available"));
    REQUIRE(report["governance"]["localizationArtifacts"].contains("issueCount"));
    REQUIRE(report["governance"]["exportArtifacts"].contains("issueCount"));

    REQUIRE(report["governance"]["assetReport"]["issueCount"] == report["assetGovernanceIssueCount"]);
    CHECK(report["schemaGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
    CHECK(report["assetGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());

    requireGovernanceSectionShape(report["governance"], "assetReport", true);
    requireGovernanceSectionShape(report["governance"], "schema", false);
    requireGovernanceSectionShape(report["governance"], "projectSchema", false);
    requireGovernanceSectionShape(report["governance"], "localizationArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "exportArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "inputArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");

    if (report["assetGovernanceIssueCount"].get<std::size_t>() > 0 ||
        report["schemaGovernanceIssueCount"].get<std::size_t>() > 0 ||
        hasPositiveIssueCount(report["governance"]["localizationArtifacts"]) ||
        hasPositiveIssueCount(report["governance"]["exportArtifacts"]) ||
        (report["governance"].contains("inputArtifacts") && hasPositiveIssueCount(report["governance"]["inputArtifacts"]))) {
        CHECK(report["summary"].get<std::string>().find("Asset governance issues:") != std::string::npos);
        CHECK(report["summary"].get<std::string>().find("Schema governance issues:") != std::string::npos);
        if (hasPositiveIssueCount(report["governance"]["localizationArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Localization artifact issues:") != std::string::npos);
        }
        if (hasPositiveIssueCount(report["governance"]["exportArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Export artifact issues:") != std::string::npos);
        }
        if (report["governance"].contains("inputArtifacts") && hasPositiveIssueCount(report["governance"]["inputArtifacts"])) {
            CHECK(report["summary"].get<std::string>().find("Input artifact issues:") != std::string::npos);
        }
    }
}

TEST_CASE("Project audit CLI selects a requested template from a synthetic readiness payload", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path tempRoot = fs::temp_directory_path() / "urpg_project_audit_cli";
    fs::create_directories(tempRoot);

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

    const ProcessResult result = runProjectAudit({"--json", "--input", inputPath.string(), "--template", "custom-template"}, repoRoot);

    REQUIRE(result.exitCode == 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const json report = json::parse(result.stdoutText);
    REQUIRE(report["templateContext"]["id"] == "custom-template");
    REQUIRE(report["templateContext"]["status"] == "READY");
    REQUIRE(report["issues"].is_array());
    REQUIRE(report["issues"].empty());
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
    REQUIRE(report["governance"].contains("exportArtifacts"));

    requireGovernanceSectionShape(report["governance"], "assetReport", true);
    requireGovernanceSectionShape(report["governance"], "schema", false);
    requireGovernanceSectionShape(report["governance"], "projectSchema", false);
    requireGovernanceSectionShape(report["governance"], "localizationArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "exportArtifacts", true);
    requireGovernanceSectionShape(report["governance"], "inputArtifacts", true);

    requireIssueCountConsistencyIfPresent(report, "inputArtifactIssueCount", "inputArtifacts");

    CHECK(report["summary"].get<std::string>().find("Selected template artifact-template is PARTIAL.") != std::string::npos);
    CHECK(report["issues"].is_array());
    CHECK(report["assetGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
    CHECK(report["schemaGovernanceIssueCount"].get<std::size_t>() <= report["issues"].size());
}

TEST_CASE("Project audit CLI fails when --input is missing its path", "[project_audit_cli]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);

    const ProcessResult result = runProjectAudit({"--json", "--input"}, repoRoot);

    REQUIRE(result.exitCode != 0);
    REQUIRE(result.stderrText.find("Missing path after --input") != std::string::npos);
}
