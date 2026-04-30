#include "project_audit_cli_test_helpers.h"

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

namespace urpg::tests::project_audit_cli {

fs::path projectAuditExecutablePath() {
    return fs::path(URPG_PROJECT_AUDIT_PATH);
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

void requireGovernanceSectionShape(const json& governance, const std::string& sectionName, bool requireIssueCount) {
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

bool jsonArrayContainsString(const json& value, const std::string& expected) {
    if (!value.is_array()) {
        return false;
    }

    for (const auto& entry : value) {
        if (entry.is_string() && entry.get<std::string>() == expected) {
            return true;
        }
    }

    return false;
}

bool reportContainsIssueCode(const json& report, const std::string& code) {
    if (!report.contains("issues") || !report["issues"].is_array()) {
        return false;
    }

    for (const auto& issue : report["issues"]) {
        if (issue.contains("code") && issue["code"].is_string() && issue["code"] == code) {
            return true;
        }
    }

    return false;
}

void writeProjectGovernanceReadinessFixture(const fs::path& path) {
    writeTextFile(path, json{
        {"schemaVersion", "1.0.0"},
        {"statusDate", "2026-04-23"},
        {"subsystems", json::array()},
        {"templates", json::array({
            {
                {"id", "governance_template"},
                {"status", "PARTIAL"},
                {"requiredSubsystems", json::array()},
                {"bars", {
                    {"input", "PARTIAL"},
                    {"localization", "PARTIAL"}
                }},
                {"mainBlockers", json::array()}
            }
        })}
    }.dump(2));
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

} // namespace urpg::tests::project_audit_cli
