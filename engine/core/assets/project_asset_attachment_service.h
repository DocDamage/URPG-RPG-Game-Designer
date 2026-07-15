#pragma once

#include "engine/core/assets/asset_promotion_manifest.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

enum class ProjectAssetAttachmentConflictPolicy { Cancel, Replace, KeepBoth, RelinkExisting };

struct ProjectAssetAttachmentPlan {
    bool valid = false;
    std::string assetId;
    std::string sourceRevision;
    std::filesystem::path payloadPath;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
};

struct ProjectAssetAttachmentRequest {
    AssetPromotionManifest manifest;
    std::filesystem::path projectRoot;
    ProjectAssetAttachmentConflictPolicy conflictPolicy = ProjectAssetAttachmentConflictPolicy::Cancel;
    // A non-empty stable ID makes a completed durable attachment idempotent.
    // ExpectedSourceRevision must come from planPromotedAssetAttachment.
    std::string operationId;
    std::string expectedSourceRevision;
};

// A transform revision remains owned by the global governed asset store until
// this request validates its immutable manifest and routes its single media
// output through the ordinary project attachment transaction. Multi-file
// revisions (for example tilesets) require their own domain assignment owner.
struct ProjectDerivedAssetAttachmentRequest {
    AssetPromotionManifest source;
    std::filesystem::path derivedManifestPath;
    std::filesystem::path projectRoot;
    ProjectAssetAttachmentConflictPolicy conflictPolicy = ProjectAssetAttachmentConflictPolicy::Cancel;
    std::string operationId;
    std::string expectedSourceRevision;
};

// Multi-file tileset revisions use a dedicated owner instead of pretending
// that their tile directory is a single attachable asset. The checked plan
// binds every expected PNG and the grid metadata to the review revision.
struct ProjectDerivedTilesetAssignmentRequest {
    AssetPromotionManifest source;
    std::filesystem::path derivedManifestPath;
    std::filesystem::path projectRoot;
    ProjectAssetAttachmentConflictPolicy conflictPolicy = ProjectAssetAttachmentConflictPolicy::Cancel;
    std::string operationId;
    std::string expectedSourceRevision;
};

struct ProjectDerivedTilesetAssignmentPlan {
    bool valid = false;
    std::string tilesetId;
    std::string sourceRevision;
    std::string derivedRevision;
    int columns = 0;
    int rows = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    std::filesystem::path tileDirectory;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
};

struct ProjectAssetAttachmentResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path payloadPath;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
    std::string sourceRevision;
    std::string operationId;
};

class ProjectAssetAttachmentService {
public:
    ProjectAssetAttachmentPlan planPromotedAssetAttachment(const AssetPromotionManifest& manifest,
                                                            const std::filesystem::path& projectRoot,
                                                            ProjectAssetAttachmentConflictPolicy conflictPolicy =
                                                                ProjectAssetAttachmentConflictPolicy::Cancel) const;
    ProjectAssetAttachmentResult attachPromotedAsset(const ProjectAssetAttachmentRequest& request) const;

    ProjectAssetAttachmentPlan planDerivedRevisionAttachment(
        const AssetPromotionManifest& source, const std::filesystem::path& derivedManifestPath,
        const std::filesystem::path& projectRoot,
        ProjectAssetAttachmentConflictPolicy conflictPolicy = ProjectAssetAttachmentConflictPolicy::Cancel) const;
    ProjectAssetAttachmentResult attachDerivedRevision(const ProjectDerivedAssetAttachmentRequest& request) const;

    ProjectDerivedTilesetAssignmentPlan planDerivedTilesetAssignment(
        const AssetPromotionManifest& source, const std::filesystem::path& derivedManifestPath,
        const std::filesystem::path& projectRoot,
        ProjectAssetAttachmentConflictPolicy conflictPolicy = ProjectAssetAttachmentConflictPolicy::Cancel) const;
    ProjectAssetAttachmentResult assignDerivedTileset(const ProjectDerivedTilesetAssignmentRequest& request) const;

    // Compatibility entry point for existing callers. New durable creator
    // actions must plan, then apply a checked ProjectAssetAttachmentRequest.
    ProjectAssetAttachmentResult attachPromotedAsset(const AssetPromotionManifest& manifest,
                                                     const std::filesystem::path& projectRoot,
                                                     ProjectAssetAttachmentConflictPolicy conflictPolicy =
                                                         ProjectAssetAttachmentConflictPolicy::Cancel) const;
};

} // namespace urpg::assets
