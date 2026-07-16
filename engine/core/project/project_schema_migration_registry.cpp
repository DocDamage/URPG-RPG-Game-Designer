#include "engine/core/project/project_schema_migration_registry.h"

#include "engine/core/migrate/migration_runner.h"

#include <algorithm>
#include <set>
#include <tuple>

namespace urpg::project {

bool ProjectSchemaMigrationRegistry::registerStep(ProjectSchemaMigrationStep step, std::string* diagnostic) {
    if (step.document_type.empty() || step.version_field.empty() || step.from_version.empty() ||
        step.to_version.empty() || step.from_version == step.to_version || !step.migration_spec.is_object() ||
        step.migration_spec.value("from", "") != step.from_version ||
        step.migration_spec.value("to", "") != step.to_version) {
        if (diagnostic) *diagnostic = "project_schema_migration_step_invalid";
        return false;
    }
    if (std::any_of(steps_.begin(), steps_.end(), [&](const auto& existing) {
            return existing.document_type == step.document_type && existing.from_version == step.from_version;
        })) {
        if (diagnostic) *diagnostic = "project_schema_migration_step_ambiguous";
        return false;
    }
    steps_.push_back(std::move(step));
    std::sort(steps_.begin(), steps_.end(), [](const auto& left, const auto& right) {
        return std::tie(left.document_type, left.from_version, left.to_version) <
               std::tie(right.document_type, right.from_version, right.to_version);
    });
    return true;
}

ProjectSchemaMigrationResult ProjectSchemaMigrationRegistry::migrate(const std::string& document_type,
                                                                     const std::string& target_version,
                                                                     nlohmann::json& document,
                                                                     const bool dry_run) const {
    ProjectSchemaMigrationResult result;
    result.dry_run = dry_run;
    result.backup = document;
    result.migrated = document;
    const auto first = std::find_if(steps_.begin(), steps_.end(), [&](const auto& step) {
        return step.document_type == document_type;
    });
    if (document_type.empty() || target_version.empty() || first == steps_.end() || !document.is_object() ||
        !document.contains(first->version_field) || !document[first->version_field].is_string()) {
        result.code = "project_schema_migration_preflight_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    const auto versionField = first->version_field;
    auto current = document[versionField].get<std::string>();
    if (current == target_version) {
        result.success = true;
        result.code = "project_schema_migration_already_current";
        return result;
    }

    std::set<std::string> visited;
    while (current != target_version) {
        if (!visited.insert(current).second) {
            result.code = "project_schema_migration_cycle";
            result.diagnostics.push_back(result.code + ":" + current);
            return result;
        }
        const auto step = std::find_if(steps_.begin(), steps_.end(), [&](const auto& candidate) {
            return candidate.document_type == document_type && candidate.version_field == versionField &&
                   candidate.from_version == current;
        });
        if (step == steps_.end()) {
            result.code = "project_schema_migration_path_missing";
            result.diagnostics.push_back(result.code + ":" + current + "->" + target_version);
            return result;
        }
        auto runnerDocument = result.migrated;
        const bool ownedRunnerVersion = versionField != "_urpg_format_version";
        runnerDocument["_urpg_format_version"] = current;
        if (const auto error = urpg::MigrationRunner::Apply(step->migration_spec, runnerDocument)) {
            result.code = "project_schema_migration_step_failed";
            result.diagnostics.push_back(result.code + ":" + error->message);
            return result;
        }
        runnerDocument[versionField] = step->to_version;
        if (ownedRunnerVersion) runnerDocument.erase("_urpg_format_version");
        result.migrated = std::move(runnerDocument);
        current = step->to_version;
        result.applied_versions.push_back(current);
    }
    result.success = true;
    result.changed = result.migrated != result.backup;
    result.code = dry_run ? "project_schema_migration_dry_run_ready" : "project_schema_migration_applied";
    if (!dry_run) document = result.migrated;
    return result;
}

bool ProjectSchemaMigrationRegistry::restoreBackup(const ProjectSchemaMigrationResult& result,
                                                   nlohmann::json& document) const {
    if (!result.success || !result.changed || !result.backup.is_object()) return false;
    document = result.backup;
    return true;
}

} // namespace urpg::project
