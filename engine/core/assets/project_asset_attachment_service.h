#pragma once

#include "engine/core/assets/asset_promotion_manifest.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

struct ProjectAssetAttachmentResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path payloadPath;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
};

class ProjectAssetAttachmentService {
public:
    ProjectAssetAttachmentResult attachPromotedAsset(const AssetPromotionManifest& manifest,
                                                     const std::filesystem::path& projectRoot) const;
};

} // namespace urpg::assets
