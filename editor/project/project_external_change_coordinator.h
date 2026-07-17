#pragma once

#include "engine/core/project/project_external_change_inspector.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace urpg::editor {

struct ProjectExternalDocumentBinding {
    std::string document_id;
    project::ProjectExternalDocumentBaseline baseline;
    std::function<std::string()> local_content;
    std::function<bool()> dirty;
    std::function<bool(const std::string&, const std::filesystem::path&, std::string&)> reload;
};

struct ProjectExternalConflict {
    std::string document_id;
    project::ProjectExternalChangeInspection inspection;
};

struct ProjectExternalResolutionResult {
    bool success = false;
    std::string code;
    std::string message;
    std::string document_id;
    project::ProjectExternalResolution resolution = project::ProjectExternalResolution::Compare;
    std::string local_content;
    std::string external_content;
    std::string previous_stable_id;
    std::string external_stable_id;
};

// Editor-session owner for external document changes. Polling only inspects;
// mutation happens after an explicit Compare, Reload, or Keep Local decision.
class ProjectExternalChangeCoordinator {
public:
    bool registerDocument(ProjectExternalDocumentBinding binding, std::string* diagnostic = nullptr);
    bool acknowledgeSaved(std::string_view document_id, std::string persisted_content,
                          std::filesystem::path document_path, std::string* diagnostic = nullptr);
    bool unregisterDocument(std::string_view document_id);
    void clear();
    std::vector<ProjectExternalConflict> inspect(
        const std::map<std::string, std::vector<std::filesystem::path>>& rename_candidates = {});
    ProjectExternalResolutionResult resolve(std::string_view document_id,
                                            project::ProjectExternalResolution resolution);
    const std::vector<ProjectExternalConflict>& conflicts() const { return conflicts_; }

private:
    ProjectExternalDocumentBinding* findBinding(std::string_view document_id);
    const ProjectExternalConflict* findConflict(std::string_view document_id) const;

    std::vector<ProjectExternalDocumentBinding> bindings_;
    std::vector<ProjectExternalConflict> conflicts_;
};

} // namespace urpg::editor
