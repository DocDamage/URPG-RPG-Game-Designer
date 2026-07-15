#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

// A bounded, read-only index of stable asset references owned by saved native
// project documents. It is an impact-analysis foundation, not a replacement
// for any document owner or attachment ledger.
struct ProjectAssetReference {
    std::string asset_id;
    std::string owner_kind;
    std::filesystem::path document_path;
    std::string local_id;
    std::string project_path;
};

struct ProjectAssetReferenceIndex {
    std::vector<ProjectAssetReference> references;
    std::vector<std::string> diagnostics;

    std::vector<ProjectAssetReference> inboundForAsset(const std::string& asset_id) const;
    std::vector<ProjectAssetReference> outboundForDocument(const std::filesystem::path& document_path) const;
};

// A read-only removal preflight. It deliberately never authorizes a deletion:
// unindexed native owners may still hold the asset ID. Domain owners must turn
// this evidence into their own validated, reversible operation.
struct ProjectAssetRemovalImpactPlan {
    std::string asset_id;
    std::vector<ProjectAssetReference> inbound_references;
    std::vector<std::string> diagnostics;
    bool is_orphan_candidate = false;
    bool removal_authorized = false;
};

ProjectAssetReferenceIndex buildPerspective2DAssetReferenceIndex(const std::filesystem::path& project_root);
ProjectAssetReferenceIndex buildProjectAssetReferenceIndex(const std::filesystem::path& project_root);
ProjectAssetRemovalImpactPlan buildProjectAssetRemovalImpactPlan(const std::filesystem::path& project_root,
                                                                  const std::string& asset_id);
std::vector<std::string> findOrphanedAttachedAssetIds(const std::filesystem::path& project_root);

} // namespace urpg::assets
