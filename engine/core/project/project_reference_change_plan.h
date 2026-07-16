#pragma once

#include "engine/core/project/project_reference_index.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::project {

enum class ProjectReferenceChangeKind { Rename, Move, Delete, Replace, Relink };

struct ProjectReferenceChangeRequest {
    std::string operation_id;
    ProjectReferenceChangeKind kind = ProjectReferenceChangeKind::Rename;
    std::string object_type;
    std::string source_id;
    std::string replacement_id;
    std::filesystem::path destination_document;
};

struct ProjectReferencePlannedUpdate {
    ProjectReferenceEdge before;
    ProjectReferenceEdge after;
};

struct ProjectReferenceChangePlan {
    bool success = false;
    bool applicable = false;
    std::string code;
    std::string message;
    ProjectReferenceChangeRequest request;
    ProjectReferenceChangeRequest inverse_request;
    std::vector<ProjectReferencePlannedUpdate> updates;
    std::vector<ProjectReferenceEdge> blocked_references;
    std::vector<ProjectReferenceEdge> package_impact;
};

// Produces a read-only, deterministic impact preview. Applying the plan remains
// the responsibility of typed document owners through ProjectOperationCoordinator.
ProjectReferenceChangePlan previewProjectReferenceChange(const ProjectReferenceIndex& index,
                                                         const ProjectReferenceChangeRequest& request);

} // namespace urpg::project
