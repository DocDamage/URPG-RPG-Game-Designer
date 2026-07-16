#pragma once

#include "engine/core/assets/asset_library.h"
#include "engine/core/project/project_operation_coordinator.h"
#include "engine/core/project/project_reference_change_plan.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

enum class ProjectAssetOperationKind { Relink, Detach, Replace, Deduplicate, Rename, Move, Delete };

struct ProjectAssetOperationRequest {
    std::string operation_id;
    ProjectAssetOperationKind kind = ProjectAssetOperationKind::Replace;
    std::string source_asset_id;
    std::string replacement_asset_id;
    std::filesystem::path destination_document;
};

struct ProjectAssetOperationPreview {
    bool success = false;
    bool applicable = false;
    std::string code;
    std::string message;
    std::string label;
    ProjectAssetOperationRequest request;
    urpg::project::ProjectReferenceChangePlan reference_plan;
};

class ProjectAssetOperationService {
public:
    ProjectAssetOperationPreview preview(const urpg::project::ProjectReferenceIndex& index,
                                         const AssetLibrary& library,
                                         const ProjectAssetOperationRequest& request) const;
    urpg::project::ProjectOperationResult execute(
        const ProjectAssetOperationPreview& preview,
        std::vector<urpg::project::ProjectOperationParticipant> participants);
    urpg::project::ProjectOperationResult undoLast();
    urpg::project::ProjectOperationResult redoLast();
    std::string undoLabel() const { return coordinator_.undoLabel(); }
    std::string redoLabel() const { return coordinator_.redoLabel(); }

private:
    urpg::project::ProjectOperationCoordinator coordinator_;
};

const char* projectAssetOperationName(ProjectAssetOperationKind kind);

} // namespace urpg::assets
