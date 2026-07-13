#include "engine/core/assets/global_asset_promotion_service.h"

#include <fstream>
#include <optional>

namespace urpg::assets {

namespace {

GlobalAssetPromotionResult failed(std::string code, std::string message, AssetPromotionManifest manifest = {}) {
    GlobalAssetPromotionResult result;
    result.success = false;
    result.code = std::move(code);
    result.message = std::move(message);
    result.manifest = std::move(manifest);
    result.diagnostics = result.manifest.diagnostics;
    return result;
}

struct ExistingPromotion {
    AssetPromotionManifest manifest;
    std::filesystem::path manifestPath;
};

std::optional<ExistingPromotion> findReusablePromotion(const std::filesystem::path& root,
                                                        const std::string& sourceSha256) {
    if (sourceSha256.empty() || !std::filesystem::is_directory(root)) {
        return std::nullopt;
    }
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator it(root, error), end; !error && it != end;
         it.increment(error)) {
        if (!it->is_regular_file() || it->path().filename() != "asset_promotion_manifest.json") {
            continue;
        }
        std::ifstream input(it->path());
        if (!input) {
            continue;
        }
        const auto manifest = deserializeAssetPromotionManifest(nlohmann::json::parse(input, nullptr, false));
        if (manifest.sourceSha256 != sourceSha256 || manifest.status != AssetPromotionStatus::RuntimeReady ||
            !manifest.package.includeInRuntime || manifest.promotedPath.empty() ||
            !std::filesystem::is_regular_file(manifest.promotedPath)) {
            continue;
        }
        return ExistingPromotion{manifest, it->path()};
    }
    return std::nullopt;
}

} // namespace

GlobalAssetPromotionResult GlobalAssetPromotionService::promoteImportRecord(
    const AssetImportSession& session, const AssetImportRecord& record, std::string licenseId,
    const std::filesystem::path& promotedRoot) const {
    auto manifest = planAssetPromotionManifest(
        session, record, std::move(licenseId), promotedRoot.generic_string(), true);
    if (manifest.status != AssetPromotionStatus::RuntimeReady || !manifest.diagnostics.empty()) {
        return failed("global_promotion_blocked", "Import record is not eligible for global promotion.", manifest);
    }

    const auto sourcePayload = std::filesystem::path(manifest.sourcePath);
    if (!std::filesystem::is_regular_file(sourcePayload)) {
        manifest.status = AssetPromotionStatus::Blocked;
        manifest.package.includeInRuntime = false;
        manifest.promotedPath.clear();
        manifest.diagnostics.push_back("source_payload_missing");
        return failed("source_payload_missing", "Import source payload file does not exist.", manifest);
    }

    if (const auto existing = findReusablePromotion(promotedRoot, manifest.sourceSha256); existing.has_value()) {
        GlobalAssetPromotionResult result;
        result.success = true;
        result.code = "global_asset_reused_by_hash";
        result.message = "An identical normalized payload is already promoted and was reused by hash.";
        result.manifest = existing->manifest;
        result.payloadPath = existing->manifest.promotedPath;
        result.manifestPath = existing->manifestPath;
        return result;
    }

    const auto destinationPayload = std::filesystem::path(manifest.promotedPath);
    const auto destinationManifest = destinationPayload.parent_path().parent_path() / "asset_promotion_manifest.json";

    std::error_code error;
    std::filesystem::create_directories(destinationPayload.parent_path(), error);
    if (error) {
        return failed("global_payload_directory_create_failed", error.message(), manifest);
    }
    std::filesystem::copy_file(sourcePayload, destinationPayload, std::filesystem::copy_options::overwrite_existing,
                               error);
    if (error) {
        return failed("global_payload_copy_failed", error.message(), manifest);
    }

    std::filesystem::create_directories(destinationManifest.parent_path(), error);
    if (error) {
        return failed("global_manifest_directory_create_failed", error.message(), manifest);
    }
    std::ofstream out(destinationManifest, std::ios::binary | std::ios::trunc);
    if (!out) {
        return failed("global_manifest_write_failed", "Global promotion manifest could not be opened.", manifest);
    }
    out << serializeAssetPromotionManifest(manifest).dump(2);
    out << '\n';
    if (!out) {
        return failed("global_manifest_write_failed", "Global promotion manifest could not be written.", manifest);
    }

    GlobalAssetPromotionResult result;
    result.success = true;
    result.code = "global_asset_promoted";
    result.message = "Import record was copied into the promoted global asset library.";
    result.manifest = std::move(manifest);
    result.payloadPath = destinationPayload;
    result.manifestPath = destinationManifest;
    return result;
}

} // namespace urpg::assets
