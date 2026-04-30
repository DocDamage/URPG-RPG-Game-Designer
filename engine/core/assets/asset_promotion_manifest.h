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
    std::string promotedPath;
    std::string licenseId;
    AssetPromotionStatus status = AssetPromotionStatus::Pending;
    AssetPromotionPreview preview;
    AssetPromotionPackage package;
    std::vector<std::string> diagnostics;
};

const char* toString(AssetPromotionStatus status);
AssetPromotionStatus assetPromotionStatusFromString(const std::string& status);

nlohmann::json serializeAssetPromotionManifest(const AssetPromotionManifest& manifest);
AssetPromotionManifest deserializeAssetPromotionManifest(const nlohmann::json& value);
std::vector<std::string> validateAssetPromotionManifest(const AssetPromotionManifest& manifest);

} // namespace urpg::assets
