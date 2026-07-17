#include "editor/project/project_external_change_coordinator.h"

#include <algorithm>
#include <array>
#include <nlohmann/json.hpp>
#include <utility>

namespace urpg::editor {
namespace {

std::string stableIdentity(const std::string& content) {
    const auto document = nlohmann::json::parse(content, nullptr, false);
    if (!document.is_object()) return {};
    constexpr std::array fields{"map_id", "quest_id", "dialogue_id", "ability_id", "character_id",
                                "vendor_id", "project_id", "id"};
    for (const auto* field : fields) {
        if (document.contains(field) && document[field].is_string()) {
            const auto value = document[field].get<std::string>();
            if (!value.empty()) return value;
        }
    }
    return {};
}

} // namespace

bool ProjectExternalChangeCoordinator::registerDocument(ProjectExternalDocumentBinding binding,
                                                        std::string* diagnostic) {
    if (binding.document_id.empty() || binding.baseline.document_path.empty() || !binding.local_content ||
        !binding.dirty || !binding.reload ||
        std::any_of(bindings_.begin(), bindings_.end(), [&](const auto& existing) {
            return existing.document_id == binding.document_id;
        })) {
        if (diagnostic) *diagnostic = "project_external_binding_invalid";
        return false;
    }
    bindings_.push_back(std::move(binding));
    std::sort(bindings_.begin(), bindings_.end(), [](const auto& left, const auto& right) {
        return left.document_id < right.document_id;
    });
    return true;
}

bool ProjectExternalChangeCoordinator::acknowledgeSaved(const std::string_view document_id,
                                                        std::string persisted_content,
                                                        std::filesystem::path document_path,
                                                        std::string* diagnostic) {
    auto* binding = findBinding(document_id);
    if (binding == nullptr || persisted_content.empty() || document_path.empty()) {
        if (diagnostic) *diagnostic = "project_external_save_acknowledgement_invalid";
        return false;
    }
    binding->baseline.document_path = std::move(document_path);
    binding->baseline.persisted_content = persisted_content;
    binding->baseline.local_content = std::move(persisted_content);
    binding->baseline.dirty = false;
    std::erase_if(conflicts_, [&](const auto& conflict) { return conflict.document_id == document_id; });
    return true;
}

bool ProjectExternalChangeCoordinator::unregisterDocument(const std::string_view document_id) {
    const auto previous = bindings_.size();
    std::erase_if(bindings_, [&](const auto& binding) { return binding.document_id == document_id; });
    std::erase_if(conflicts_, [&](const auto& conflict) { return conflict.document_id == document_id; });
    return bindings_.size() != previous;
}

void ProjectExternalChangeCoordinator::clear() {
    bindings_.clear();
    conflicts_.clear();
}

std::vector<ProjectExternalConflict> ProjectExternalChangeCoordinator::inspect(
    const std::map<std::string, std::vector<std::filesystem::path>>& rename_candidates) {
    std::vector<ProjectExternalConflict> next;
    for (auto& binding : bindings_) {
        binding.baseline.local_content = binding.local_content();
        binding.baseline.dirty = binding.dirty();
        const auto candidates = rename_candidates.find(binding.document_id);
        auto inspection = project::inspectProjectExternalChange(
            binding.baseline, candidates == rename_candidates.end() ? std::vector<std::filesystem::path>{}
                                                                    : candidates->second);
        if (!inspection.success || inspection.kind == project::ProjectExternalChangeKind::Unchanged) continue;
        next.push_back({binding.document_id, std::move(inspection)});
    }
    conflicts_ = std::move(next);
    return conflicts_;
}

ProjectExternalResolutionResult ProjectExternalChangeCoordinator::resolve(
    const std::string_view document_id, const project::ProjectExternalResolution resolution) {
    ProjectExternalResolutionResult result;
    result.document_id = document_id;
    result.resolution = resolution;
    auto* binding = findBinding(document_id);
    const auto* conflict = findConflict(document_id);
    if (binding == nullptr || conflict == nullptr) {
        result.code = "project_external_conflict_missing";
        result.message = "The external change is no longer available for resolution.";
        return result;
    }
    if (std::find(conflict->inspection.available_resolutions.begin(),
                  conflict->inspection.available_resolutions.end(), resolution) ==
        conflict->inspection.available_resolutions.end()) {
        result.code = "project_external_resolution_unavailable";
        result.message = "That resolution is unsafe for the observed external change.";
        return result;
    }
    result.local_content = binding->local_content();
    result.external_content = conflict->inspection.observed_content;
    if (resolution == project::ProjectExternalResolution::Compare) {
        result.success = true;
        result.code = "project_external_compare_ready";
        result.message = "Local and external content are ready for comparison; neither was changed.";
        return result;
    }
    if (resolution == project::ProjectExternalResolution::Reload) {
        if (conflict->inspection.kind == project::ProjectExternalChangeKind::Renamed) {
            result.previous_stable_id = stableIdentity(binding->baseline.persisted_content);
            result.external_stable_id = stableIdentity(conflict->inspection.observed_content);
            if (!result.previous_stable_id.empty() && !result.external_stable_id.empty() &&
                result.previous_stable_id != result.external_stable_id) {
                result.code = "project_external_identity_impact_review_required";
                result.message = "The renamed document also changes stable identity from '" +
                                 result.previous_stable_id + "' to '" + result.external_stable_id +
                                 "'. Review inbound and outbound reference impact before applying it.";
                return result;
            }
        }
        std::string diagnostic;
        if (!binding->reload(conflict->inspection.observed_content, conflict->inspection.observed_path, diagnostic)) {
            result.code = "project_external_reload_failed";
            result.message = diagnostic.empty() ? "The document owner rejected the external content." : diagnostic;
            return result;
        }
        binding->baseline.document_path = conflict->inspection.observed_path;
        binding->baseline.persisted_content = conflict->inspection.observed_content;
        binding->baseline.local_content = conflict->inspection.observed_content;
        result.code = "project_external_reloaded";
        result.message = "The validated external document replaced the local draft.";
    } else {
        // Acknowledge what is currently on disk while retaining the owner's
        // local draft as dirty work. A later save remains an explicit action.
        binding->baseline.document_path = conflict->inspection.observed_path;
        binding->baseline.persisted_content = conflict->inspection.observed_content;
        binding->baseline.local_content = binding->local_content();
        result.code = "project_external_local_kept";
        result.message = "The local draft was preserved and remains unsaved.";
    }
    result.success = true;
    std::erase_if(conflicts_, [&](const auto& item) { return item.document_id == document_id; });
    return result;
}

ProjectExternalDocumentBinding* ProjectExternalChangeCoordinator::findBinding(const std::string_view document_id) {
    const auto found = std::find_if(bindings_.begin(), bindings_.end(), [&](const auto& binding) {
        return binding.document_id == document_id;
    });
    return found == bindings_.end() ? nullptr : &*found;
}

const ProjectExternalConflict* ProjectExternalChangeCoordinator::findConflict(const std::string_view document_id) const {
    const auto found = std::find_if(conflicts_.begin(), conflicts_.end(), [&](const auto& conflict) {
        return conflict.document_id == document_id;
    });
    return found == conflicts_.end() ? nullptr : &*found;
}

} // namespace urpg::editor
