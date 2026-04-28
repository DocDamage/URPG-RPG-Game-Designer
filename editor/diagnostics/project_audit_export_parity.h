#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::editor {

struct ProjectAuditExportParityIssue {
    std::string code;
    std::string path;
    nlohmann::json cli_value;
    nlohmann::json workspace_value;
};

struct ProjectAuditExportParityResult {
    bool matches = false;
    nlohmann::json cli_canonical;
    nlohmann::json workspace_canonical;
    std::vector<ProjectAuditExportParityIssue> issues;
};

nlohmann::json buildProjectAuditCliCanonical(const nlohmann::json& cli_report);
nlohmann::json buildProjectAuditWorkspaceCanonical(const nlohmann::json& workspace_export);
ProjectAuditExportParityResult compareProjectAuditExportParity(const nlohmann::json& cli_report,
                                                               const nlohmann::json& workspace_export);
nlohmann::json projectAuditExportParityIssueToJson(const ProjectAuditExportParityIssue& issue);
nlohmann::json projectAuditExportParityResultToJson(const ProjectAuditExportParityResult& result);

} // namespace urpg::editor
