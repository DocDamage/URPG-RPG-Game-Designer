#pragma once

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

} // namespace urpg::asset
