#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::project {

struct ProjectSchemaMigrationStep {
    std::string document_type;
    std::string version_field;
    std::string from_version;
    std::string to_version;
    nlohmann::json migration_spec;
};

struct ProjectSchemaMigrationResult {
    bool success = false;
    bool dry_run = false;
    bool changed = false;
    std::string code;
    std::vector<std::string> applied_versions;
    std::vector<std::string> diagnostics;
    nlohmann::json backup;
    nlohmann::json migrated;
};

class ProjectSchemaMigrationRegistry {
public:
    bool registerStep(ProjectSchemaMigrationStep step, std::string* diagnostic = nullptr);
    ProjectSchemaMigrationResult migrate(const std::string& document_type, const std::string& target_version,
                                         nlohmann::json& document, bool dry_run) const;
    bool restoreBackup(const ProjectSchemaMigrationResult& result, nlohmann::json& document) const;

private:
    std::vector<ProjectSchemaMigrationStep> steps_;
};

} // namespace urpg::project
