#pragma once

#include "engine/core/assets/asset_import_session.h"
#include "engine/core/assets/asset_promotion_manifest.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

struct GlobalAssetPromotionResult {
    bool success = false;
    std::string code;
    std::string message;
    AssetPromotionManifest manifest;
    std::filesystem::path payloadPath;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
};

class GlobalAssetPromotionService {
public:
    GlobalAssetPromotionResult promoteImportRecord(const AssetImportSession& session, const AssetImportRecord& record,
                                                   std::string licenseId,
                                                   const std::filesystem::path& promotedRoot) const;
};

} // namespace urpg::assets
