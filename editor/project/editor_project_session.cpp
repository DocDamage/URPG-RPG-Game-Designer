#include "editor/project/editor_project_session.h"

#include "engine/core/project/project_document_migration_service.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <system_error>
#include <utility>

namespace urpg::editor {

EditorProjectSessionResult EditorProjectSession::validateProject(const std::filesystem::path& project_root,
                                                                  EditorProjectIdentity& identity) const {
    std::error_code error;
    if (!std::filesystem::is_directory(project_root, error) || error) {
        return {false, "project_root_missing", "Project root does not exist or is not a directory: " + project_root.string()};
    }

    const auto manifest_path = project_root / "project.json";
    if (!std::filesystem::is_regular_file(manifest_path, error) || error) {
        return {false, "project_manifest_missing", "Project root must contain a readable project.json manifest."};
    }

    std::ifstream input(manifest_path, std::ios::binary);
    const auto manifest = nlohmann::json::parse(input, nullptr, false);
    if (manifest.is_discarded() || !manifest.is_object()) {
        return {false, "project_manifest_invalid", "Project manifest must contain a JSON object."};
    }

    const auto project_id = manifest.value("project_id", "");
    const auto display_name = manifest.value("project_name", "");
    const auto schema_version = manifest.value("schema_version", "");
    if (project_id.empty() || display_name.empty() || schema_version.empty()) {
        return {false, "project_manifest_incomplete",
                "Project manifest requires schema_version, project_id, and project_name before it can be opened."};
    }

    identity.root = std::filesystem::weakly_canonical(project_root, error);
    if (error) {
        return {false, "project_root_unresolvable", "Project root could not be normalized: " + error.message()};
    }
    identity.project_id = project_id;
    identity.display_name = display_name;
    identity.schema_version = schema_version;
    return {true, "project_opened", "Project opened successfully."};
}

EditorProjectSessionResult EditorProjectSession::openProject(const std::filesystem::path& project_root) {
    auto inspection = inspectProject(project_root);
    auto result = inspection.result;
    if (!result.success) {
        last_diagnostic_ = result;
        return result;
    }

    const auto migration = urpg::project::migrateProjectDocumentsOnDisk(inspection.identity.root);
    if (!migration.success) {
        result = {false, migration.code,
                  "Project documents could not be migrated safely before open; no partial migration was retained."};
        last_diagnostic_ = result;
        return result;
    }
    if (migration.code == "project_document_migration_applied") {
        result.code = "project_opened_after_migration";
        result.message = "Project opened successfully after forward migration; originals were retained privately.";
    }

    if (open_) {
        for (const auto& listener : close_listeners_) listener(active_project_);
    }
    active_project_ = std::move(inspection.identity);
    open_ = true;
    last_diagnostic_ = result;
    for (const auto& listener : switch_listeners_) {
        listener(active_project_);
    }
    return result;
}

EditorProjectInspection EditorProjectSession::inspectProject(const std::filesystem::path& project_root) const {
    EditorProjectInspection inspection;
    inspection.result = validateProject(project_root, inspection.identity);
    return inspection;
}

EditorProjectSessionResult EditorProjectSession::closeProject() {
    if (open_) {
        for (const auto& listener : close_listeners_) listener(active_project_);
    }
    active_project_ = {};
    dirty_surface_summaries_.clear();
    open_ = false;
    last_diagnostic_ = {true, "project_closed", "Project closed."};
    return last_diagnostic_;
}

void EditorProjectSession::setDirtySurfaceSummaries(std::vector<std::string> summaries) {
    dirty_surface_summaries_ = std::move(summaries);
}

void EditorProjectSession::addSwitchListener(SwitchListener listener) {
    if (listener) {
        switch_listeners_.push_back(std::move(listener));
    }
}

void EditorProjectSession::addCloseListener(CloseListener listener) {
    if (listener) {
        close_listeners_.push_back(std::move(listener));
    }
}

} // namespace urpg::editor
