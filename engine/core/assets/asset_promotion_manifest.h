#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::assets {

enum class AssetPromotionStatus {
    Pending,
    RuntimeReady,
    Blocked,
    Archived,
};

struct AssetPromotionPreview {
    std::string kind = "none";
    std::string thumbnailPath;
    int32_t width = 0;
    int32_t height = 0;
};

struct AssetPromotionPackage {
    bool includeInRuntime = false;
    bool requiredForRelease = false;
};

struct AssetPromotionManifest {
    std::string schemaVersion = "1.0.0";
    std::string assetId;
    std::string sourcePath;
    // Optional immutable content identity captured by the import review. It
    // enables safe reuse of an already-promoted normalized payload.
    std::string sourceSha256;
    std::string promotedPath;
    std::string licenseId;
    AssetPromotionStatus status = AssetPromotionStatus::Pending;
    AssetPromotionPreview preview;
    AssetPromotionPackage package;
    nlohmann::json authoredMetadata = nlohmann::json::object();
    std::vector<std::string> diagnostics;
};

const char* toString(AssetPromotionStatus status);
AssetPromotionStatus assetPromotionStatusFromString(const std::string& status);

nlohmann::json serializeAssetPromotionManifest(const AssetPromotionManifest& manifest);
AssetPromotionManifest deserializeAssetPromotionManifest(const nlohmann::json& value);
std::vector<std::string> validateAssetPromotionManifest(const AssetPromotionManifest& manifest);

} // namespace urpg::assets
