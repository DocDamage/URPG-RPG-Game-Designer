#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace urpg::asset {

/**
 * @brief Common license types for assets.
 */
enum class LicenseType {
    Unknown,
    Proprietary,
    CreativeCommons_BY,
    CreativeCommons_BY_SA,
    MIT,
    PublicDomain
};

/**
 * @brief Metadata for asset licensing.
 */
struct AssetLicense {
    LicenseType type = LicenseType::Unknown;
    std::string attribution; // Required for CC-BY etc.
    std::string sourceUrl;
};

/**
 * @brief A scan result for a single asset.
 */
struct AssetAuditResult {
    std::string filePath;
    AssetLicense license;
    bool isExportSafe = false;
    std::string warningMessage;
};

/**
 * @brief Baseline for asset license auditing.
 * Required for Phase 4 Ecosystem security and export safety.
 */
class AssetLicenseAuditor {
public:
    static AssetAuditResult auditAsset(const std::string& path, const AssetLicense& declaredLicense) {
        AssetAuditResult result;
        result.filePath = path;
        result.license = declaredLicense;

        if (declaredLicense.type == LicenseType::Unknown) {
            result.isExportSafe = false;
            result.warningMessage = "Asset has no declared license and cannot be exported safely.";
        } else if (declaredLicense.type == LicenseType::Proprietary) {
            result.isExportSafe = true; // Assuming internal proprietary is okay
            result.warningMessage = "Proprietary asset: ensure project has distribution rights.";
        } else {
            result.isExportSafe = true;
        }

        return result;
    }
};

struct ReleaseAssetManifestIssue {
    std::string code;
    std::string path;
};

struct ReleaseAssetManifestResult {
    bool exportSafe = false;
    std::size_t releaseRequiredCount = 0;
    std::size_t bundledCount = 0;
    std::vector<ReleaseAssetManifestIssue> issues;
};

inline bool isRawOrVendorAssetPath(const std::string& path) {
    return path.find("third_party/") == 0 || path.find("third_party\\") == 0 ||
           path.find("vendor/") == 0 || path.find("vendor\\") == 0 ||
           path.find("more assets/") == 0 || path.find("more assets\\") == 0 ||
           path.find("/PSD/") != std::string::npos || path.find("\\PSD\\") != std::string::npos;
}

inline ReleaseAssetManifestResult auditReleaseAssetManifest(const nlohmann::json& manifest) {
    ReleaseAssetManifestResult result;
    if (!manifest.is_object() || !manifest.contains("assets") || !manifest["assets"].is_array()) {
        result.issues.push_back({"malformed_manifest", ""});
        return result;
    }

    for (const auto& asset : manifest["assets"]) {
        const auto path = asset.value("path", "");
        if (path.empty()) {
            result.issues.push_back({"missing_path", ""});
            continue;
        }
        if (!asset.value("release_required", false)) {
            continue;
        }

        ++result.releaseRequiredCount;
        if (asset.value("distribution", "") == "bundled") {
            ++result.bundledCount;
        } else {
            result.issues.push_back({"release_asset_not_bundled", path});
        }
        if (!asset.value("license_cleared", false)) {
            result.issues.push_back({"license_not_cleared", path});
        }
        if (asset.value("attribution", "").empty()) {
            result.issues.push_back({"missing_attribution", path});
        }
        if (asset.value("source_url", "").empty()) {
            result.issues.push_back({"missing_source_url", path});
        }
        if (isRawOrVendorAssetPath(path)) {
            result.issues.push_back({"raw_or_vendor_release_path", path});
        }
    }

    result.exportSafe = result.releaseRequiredCount > 0 && result.issues.empty();
    return result;
}

} // namespace urpg::asset
