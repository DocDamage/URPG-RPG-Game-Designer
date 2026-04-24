#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class ProjectHealthSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

enum class ProjectHealthGroup : uint8_t {
    ReleaseBlockers = 0,
    ExportBlockers = 1,
    Warnings = 2,
    GovernanceIssues = 3,
    AssetIssues = 4,
    SchemaIssues = 5,
};

struct ProjectHealthFixCard {
    std::string code;
    std::string title;
    std::string detail;
    std::string owning_subsystem;
    ProjectHealthSeverity severity = ProjectHealthSeverity::Info;
    ProjectHealthGroup group = ProjectHealthGroup::Warnings;
    bool blocks_release = false;
    bool blocks_export = false;
    std::vector<std::string> affected_paths;
    std::vector<std::string> validation_commands;
    std::vector<std::string> acceptance_criteria;
};

struct ProjectHealthGroupCard {
    ProjectHealthGroup group = ProjectHealthGroup::Warnings;
    std::string title;
    size_t issue_count = 0;
    size_t blocker_count = 0;
    std::vector<ProjectHealthFixCard> fixes;
};

struct ProjectHealthSnapshot {
    bool has_data = false;
    bool stale = false;
    std::string headline;
    std::string summary;
    std::string status_date;
    std::string readiness_date;
    size_t issue_count = 0;
    size_t release_blocker_count = 0;
    size_t export_blocker_count = 0;
    std::vector<ProjectHealthGroupCard> groups;
    std::vector<ProjectHealthFixCard> fix_next;
};

class ProjectHealthModel {
public:
    void ingestAuditReport(const nlohmann::json& report);
    void clear();

    const ProjectHealthSnapshot& snapshot() const { return snapshot_; }
    bool hasReportData() const { return snapshot_.has_data; }

private:
    ProjectHealthSnapshot snapshot_{};
};

const char* toString(ProjectHealthSeverity severity);
const char* toString(ProjectHealthGroup group);

} // namespace urpg::editor
