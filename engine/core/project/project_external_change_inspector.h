#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::project {

enum class ProjectExternalChangeKind { Unchanged, Modified, Renamed, Deleted, Malformed };
enum class ProjectExternalResolution { Compare, Reload, KeepLocal };

struct ProjectExternalDocumentBaseline {
    std::filesystem::path document_path;
    std::string persisted_content;
    std::string local_content;
    bool dirty = false;
    bool require_valid_json = true;
};

struct ProjectExternalChangeInspection {
    bool success = false;
    std::string code;
    ProjectExternalChangeKind kind = ProjectExternalChangeKind::Unchanged;
    std::filesystem::path original_path;
    std::filesystem::path observed_path;
    std::string observed_content;
    bool has_unsaved_conflict = false;
    std::vector<ProjectExternalResolution> available_resolutions;
    std::vector<std::string> diagnostics;
};

ProjectExternalChangeInspection inspectProjectExternalChange(
    const ProjectExternalDocumentBaseline& baseline,
    const std::vector<std::filesystem::path>& rename_candidates = {});

} // namespace urpg::project
