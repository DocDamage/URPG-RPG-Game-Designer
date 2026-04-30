#include "engine/core/assets/asset_promotion_manifest.h"

namespace urpg::assets {

const char* toString(AssetPromotionStatus status) {
    switch (status) {
    case AssetPromotionStatus::Pending:
        return "pending";
    case AssetPromotionStatus::RuntimeReady:
        return "runtime_ready";
    case AssetPromotionStatus::Blocked:
        return "blocked";
    case AssetPromotionStatus::Archived:
        return "archived";
    }
    return "pending";
}

AssetPromotionStatus assetPromotionStatusFromString(const std::string& status) {
    if (status == "runtime_ready") {
        return AssetPromotionStatus::RuntimeReady;
    }
    if (status == "blocked") {
        return AssetPromotionStatus::Blocked;
    }
    if (status == "archived") {
        return AssetPromotionStatus::Archived;
    }
    return AssetPromotionStatus::Pending;
}

nlohmann::json serializeAssetPromotionManifest(const AssetPromotionManifest& manifest) {
    return {
        {"schemaVersion", manifest.schemaVersion},
        {"assetId", manifest.assetId},
        {"sourcePath", manifest.sourcePath},
        {"promotedPath", manifest.promotedPath},
        {"licenseId", manifest.licenseId},
        {"status", toString(manifest.status)},
        {"preview",
         {
             {"kind", manifest.preview.kind},
             {"thumbnailPath", manifest.preview.thumbnailPath},
             {"width", manifest.preview.width},
             {"height", manifest.preview.height},
         }},
        {"package",
         {
             {"includeInRuntime", manifest.package.includeInRuntime},
             {"requiredForRelease", manifest.package.requiredForRelease},
         }},
        {"diagnostics", manifest.diagnostics},
    };
}

std::vector<std::string> validateAssetPromotionManifest(const AssetPromotionManifest& manifest) {
    std::vector<std::string> diagnostics;
    if (manifest.assetId.empty()) {
        diagnostics.push_back("asset_id_missing");
    }
    if (manifest.status == AssetPromotionStatus::RuntimeReady && manifest.promotedPath.empty()) {
        diagnostics.push_back("promoted_path_missing");
    }
    if (manifest.package.includeInRuntime && manifest.licenseId.empty()) {
        diagnostics.push_back("license_evidence_missing");
    }
    if (manifest.package.requiredForRelease && !manifest.package.includeInRuntime) {
        diagnostics.push_back("release_required_not_packaged");
    }
    if (manifest.preview.kind != "none" && manifest.preview.kind != "pending" && manifest.preview.thumbnailPath.empty()) {
        diagnostics.push_back("preview_thumbnail_missing");
    }
    if (manifest.status == AssetPromotionStatus::Archived && manifest.package.includeInRuntime) {
        diagnostics.push_back("archived_asset_packaged");
    }
    return diagnostics;
}

AssetPromotionManifest deserializeAssetPromotionManifest(const nlohmann::json& value) {
    AssetPromotionManifest manifest;
    if (!value.is_object()) {
        manifest.diagnostics.push_back("manifest_not_object");
        return manifest;
    }

    manifest.schemaVersion = value.value("schemaVersion", manifest.schemaVersion);
    manifest.assetId = value.value("assetId", "");
    manifest.sourcePath = value.value("sourcePath", "");
    manifest.promotedPath = value.value("promotedPath", "");
    manifest.licenseId = value.value("licenseId", "");
    manifest.status = assetPromotionStatusFromString(value.value("status", "pending"));

    const auto preview = value.find("preview");
    if (preview != value.end() && preview->is_object()) {
        manifest.preview.kind = preview->value("kind", manifest.preview.kind);
        manifest.preview.thumbnailPath = preview->value("thumbnailPath", "");
        manifest.preview.width = preview->value("width", 0);
        manifest.preview.height = preview->value("height", 0);
    }

    const auto package = value.find("package");
    if (package != value.end() && package->is_object()) {
        manifest.package.includeInRuntime = package->value("includeInRuntime", false);
        manifest.package.requiredForRelease = package->value("requiredForRelease", false);
    }

    const auto existingDiagnostics = value.find("diagnostics");
    if (existingDiagnostics != value.end() && existingDiagnostics->is_array()) {
        for (const auto& diagnostic : *existingDiagnostics) {
            if (diagnostic.is_string()) {
                manifest.diagnostics.push_back(diagnostic.get<std::string>());
            }
        }
    }

    auto validationDiagnostics = validateAssetPromotionManifest(manifest);
    manifest.diagnostics.insert(
        manifest.diagnostics.end(), validationDiagnostics.begin(), validationDiagnostics.end());
    return manifest;
}

} // namespace urpg::assets
