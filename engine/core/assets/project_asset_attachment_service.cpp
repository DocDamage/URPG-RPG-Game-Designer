#include "engine/core/assets/project_asset_attachment_service.h"

#include <algorithm>
#include <fstream>

namespace urpg::assets {

namespace {

std::string sanitizeSegment(std::string value) {
    for (auto& ch : value) {
        const bool keep = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
                          ch == '-' || ch == '_' || ch == '.';
        if (!keep) {
            ch = '-';
        }
    }
    return value.empty() ? "asset" : value;
}

bool pathInside(const std::filesystem::path& root, const std::filesystem::path& child) {
    const auto rootNormalized = std::filesystem::weakly_canonical(root);
    const auto childNormalized = std::filesystem::weakly_canonical(child);
    return childNormalized == rootNormalized ||
           std::mismatch(rootNormalized.begin(), rootNormalized.end(), childNormalized.begin()).first ==
               rootNormalized.end();
}

ProjectAssetAttachmentResult blocked(std::string code, std::string message, std::vector<std::string> diagnostics = {}) {
    ProjectAssetAttachmentResult result;
    result.success = false;
    result.code = std::move(code);
    result.message = std::move(message);
    result.diagnostics = std::move(diagnostics);
    return result;
}

} // namespace

ProjectAssetAttachmentResult ProjectAssetAttachmentService::attachPromotedAsset(
    const AssetPromotionManifest& manifest, const std::filesystem::path& projectRoot) const {
    auto diagnostics = validateAssetPromotionManifest(manifest);
    diagnostics.insert(diagnostics.end(), manifest.diagnostics.begin(), manifest.diagnostics.end());
    if (!diagnostics.empty()) {
        return blocked("asset_promotion_invalid", "Promoted asset manifest has unresolved diagnostics.", diagnostics);
    }
    if (manifest.status != AssetPromotionStatus::RuntimeReady || !manifest.package.includeInRuntime) {
        return blocked("asset_not_runtime_ready", "Only runtime-ready promoted assets can be attached to a project.");
    }
    if (manifest.promotedPath.empty()) {
        return blocked("promoted_payload_missing", "Promoted asset payload path is empty.");
    }

    const auto sourcePayload = std::filesystem::path(manifest.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePayload)) {
        return blocked("promoted_payload_missing", "Promoted asset payload file does not exist.");
    }

    const auto projectContent = projectRoot / "content";
    const auto importedRoot = projectContent / "assets" / "imported";
    const auto manifestRoot = projectContent / "assets" / "manifests";
    const auto assetSegment = sanitizeSegment(manifest.assetId);
    const auto destinationPayload = importedRoot / assetSegment / sourcePayload.filename();
    const auto destinationManifest = manifestRoot / (assetSegment + ".json");

    std::error_code error;
    std::filesystem::create_directories(destinationPayload.parent_path(), error);
    if (error) {
        return blocked("project_asset_directory_create_failed", error.message());
    }
    std::filesystem::create_directories(destinationManifest.parent_path(), error);
    if (error) {
        return blocked("project_manifest_directory_create_failed", error.message());
    }
    if (!pathInside(projectContent, destinationPayload) || !pathInside(projectContent, destinationManifest)) {
        return blocked("project_attachment_path_escape", "Project attachment destination escaped the project content root.");
    }

    std::filesystem::copy_file(sourcePayload, destinationPayload, std::filesystem::copy_options::overwrite_existing,
                               error);
    if (error) {
        return blocked("project_asset_copy_failed", error.message());
    }

    auto projectManifest = manifest;
    projectManifest.sourcePath = manifest.promotedPath;
    projectManifest.promotedPath = destinationPayload.generic_string();
    projectManifest.preview.thumbnailPath =
        manifest.preview.kind == "image" || manifest.preview.kind == "audio" ? destinationPayload.generic_string()
                                                                              : manifest.preview.thumbnailPath;
    projectManifest.package.includeInRuntime = true;
    projectManifest.package.requiredForRelease = manifest.package.requiredForRelease;
    projectManifest.diagnostics.clear();

    std::ofstream out(destinationManifest, std::ios::binary | std::ios::trunc);
    if (!out) {
        return blocked("project_manifest_write_failed", "Project asset manifest could not be opened for writing.");
    }
    out << serializeAssetPromotionManifest(projectManifest).dump(2);
    out << '\n';
    if (!out) {
        return blocked("project_manifest_write_failed", "Project asset manifest could not be written.");
    }

    ProjectAssetAttachmentResult result;
    result.success = true;
    result.code = "project_asset_attached";
    result.message = "Promoted asset was attached to the project.";
    result.payloadPath = destinationPayload;
    result.manifestPath = destinationManifest;
    return result;
}

} // namespace urpg::assets
