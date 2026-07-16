#include "engine/core/project/project_external_change_inspector.h"

#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

namespace urpg::project {
namespace {

bool readFile(const std::filesystem::path& path, std::string& content) {
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) return false;
    content.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.good() || input.eof();
}

bool validJson(const std::string& content) {
    return !nlohmann::json::parse(content, nullptr, false).is_discarded();
}

} // namespace

ProjectExternalChangeInspection inspectProjectExternalChange(
    const ProjectExternalDocumentBaseline& baseline,
    const std::vector<std::filesystem::path>& rename_candidates) {
    ProjectExternalChangeInspection result;
    result.original_path = baseline.document_path.lexically_normal();
    result.observed_path = result.original_path;
    if (baseline.document_path.empty()) {
        result.code = "project_external_change_path_missing";
        result.diagnostics.push_back(result.code);
        return result;
    }

    std::string observed;
    if (!readFile(baseline.document_path, observed)) {
        for (const auto& candidate : rename_candidates) {
            std::string candidateContent;
            if (candidate.lexically_normal() != result.original_path && readFile(candidate, candidateContent) &&
                candidateContent == baseline.persisted_content) {
                result.success = true;
                result.kind = ProjectExternalChangeKind::Renamed;
                result.code = "project_external_change_renamed";
                result.observed_path = candidate.lexically_normal();
                result.observed_content = std::move(candidateContent);
                result.has_unsaved_conflict = baseline.dirty;
                result.available_resolutions = {ProjectExternalResolution::Compare, ProjectExternalResolution::KeepLocal,
                                                ProjectExternalResolution::Reload};
                return result;
            }
        }
        result.success = true;
        result.kind = ProjectExternalChangeKind::Deleted;
        result.code = "project_external_change_deleted";
        result.has_unsaved_conflict = baseline.dirty;
        result.available_resolutions = {ProjectExternalResolution::Compare, ProjectExternalResolution::KeepLocal};
        return result;
    }

    result.observed_content = std::move(observed);
    if (result.observed_content == baseline.persisted_content) {
        result.success = true;
        result.kind = ProjectExternalChangeKind::Unchanged;
        result.code = "project_external_change_unchanged";
        return result;
    }
    result.has_unsaved_conflict = baseline.dirty && baseline.local_content != result.observed_content;
    result.available_resolutions = {ProjectExternalResolution::Compare, ProjectExternalResolution::KeepLocal,
                                    ProjectExternalResolution::Reload};
    if (baseline.require_valid_json && !validJson(result.observed_content)) {
        result.success = true;
        result.kind = ProjectExternalChangeKind::Malformed;
        result.code = "project_external_change_malformed";
        result.diagnostics.push_back("External JSON is malformed; reload must remain unavailable until explicitly repaired.");
        result.available_resolutions = {ProjectExternalResolution::Compare, ProjectExternalResolution::KeepLocal};
        return result;
    }
    result.success = true;
    result.kind = ProjectExternalChangeKind::Modified;
    result.code = result.has_unsaved_conflict ? "project_external_change_dirty_conflict"
                                              : "project_external_change_modified";
    return result;
}

} // namespace urpg::project
