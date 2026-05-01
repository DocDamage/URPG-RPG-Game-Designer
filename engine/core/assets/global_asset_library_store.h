#pragma once

#include "engine/core/assets/asset_import_session.h"
#include "engine/core/assets/asset_promotion_manifest.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

struct GlobalAssetLibraryStoreResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path path;
};

class GlobalAssetLibraryStore {
public:
    explicit GlobalAssetLibraryStore(std::filesystem::path libraryRoot);

    const std::filesystem::path& libraryRoot() const { return library_root_; }
    std::filesystem::path catalogDatabasePath() const;
    std::filesystem::path importSessionManifestPath(std::string_view sessionId) const;
    std::filesystem::path sourceManifestPath(std::string_view sessionId) const;
    std::filesystem::path promotedAssetManifestPath(std::string_view assetId) const;

    GlobalAssetLibraryStoreResult writeImportSession(const AssetImportSession& session) const;
    GlobalAssetLibraryStoreResult writePromotedAssetManifest(const AssetPromotionManifest& manifest) const;
    std::vector<AssetImportSession> loadImportSessions() const;
    std::vector<AssetPromotionManifest> loadPromotedAssetManifests() const;

private:
    std::filesystem::path library_root_;
};

} // namespace urpg::assets
