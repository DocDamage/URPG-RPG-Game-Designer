#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::tests::project_audit_cli {

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
};

fs::path projectAuditExecutablePath();
std::string readTextFile(const fs::path& path);
void writeTextFile(const fs::path& path, const std::string& content);
void requireGovernanceSectionShape(const json& governance, const std::string& sectionName,
                                   bool requireIssueCount = false);
bool hasPositiveIssueCount(const json& section);
void requireIssueCountConsistencyIfPresent(const json& report, const std::string& topLevelKey,
                                           const std::string& sectionKey);
bool jsonArrayContainsString(const json& value, const std::string& expected);
bool reportContainsIssueCode(const json& report, const std::string& code);
void writeProjectGovernanceReadinessFixture(const fs::path& path);
ProcessResult runProjectAudit(const std::vector<std::string>& args, const fs::path& workDir);

} // namespace urpg::tests::project_audit_cli
