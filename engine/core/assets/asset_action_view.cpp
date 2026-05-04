#include "engine/core/assets/asset_action_view.h"

#include <algorithm>

namespace urpg::assets {

namespace {

bool hasStatus(const AssetRecord& asset, AssetStatus status) {
    return asset.statuses.contains(status);
}

nlohmann::json statusList(const AssetRecord& asset) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto status : asset.statuses) {
        out.push_back(toString(status));
    }
    return out;
}

std::string promoteBlockReason(const AssetRecord& asset) {
    if (asset.path.empty()) {
        return "asset_missing";
    }
    if (hasStatus(asset, AssetStatus::Promoted)) {
        return "asset_already_promoted";
    }
    if (asset.normalized_path.empty()) {
        return "asset_not_normalized";
    }
    if (hasStatus(asset, AssetStatus::MissingFile)) {
        return "asset_missing_file";
    }
    if (hasStatus(asset, AssetStatus::UnsupportedFormat)) {
        return "asset_unsupported_format";
    }
    if (hasStatus(asset, AssetStatus::MissingLicense)) {
        return "asset_missing_license";
    }
    if (hasStatus(asset, AssetStatus::Duplicate)) {
        return "asset_duplicate";
    }
    if (hasStatus(asset, AssetStatus::Archived)) {
        return "asset_archived";
    }
    return {};
}

std::string archiveBlockReason(const AssetRecord& asset) {
    if (asset.path.empty()) {
        return "asset_missing";
    }
    if (hasStatus(asset, AssetStatus::Archived)) {
        return "asset_already_archived";
    }
    if (!asset.used_by.empty()) {
        return "asset_in_use";
    }
    return {};
}

bool isProjectAttached(const AssetRecord& asset) {
    return std::any_of(asset.used_by.begin(), asset.used_by.end(),
                       [](const auto& owner) { return owner.rfind("project_asset_attachment:", 0) == 0; });
}

std::string attachBlockReason(const AssetRecord& asset) {
    if (asset.path.empty()) {
        return "asset_missing";
    }
    if (isProjectAttached(asset)) {
        return "asset_already_attached";
    }
    if (!hasStatus(asset, AssetStatus::Promoted) || asset.promotion_status != "runtime_ready") {
        return "asset_not_promoted";
    }
    if (!asset.include_in_runtime) {
        return "asset_not_runtime_packageable";
    }
    if (asset.promoted_path.empty()) {
        return "promoted_path_missing";
    }
    if (!asset.promotion_diagnostics.empty()) {
        return "asset_promotion_blocked";
    }
    return {};
}

std::string recommendedAction(const AssetRecord& asset, bool canPromote, bool canArchive, bool canAttach) {
    if (hasStatus(asset, AssetStatus::MissingFile)) {
        return "fix_missing_file";
    }
    if (hasStatus(asset, AssetStatus::UnsupportedFormat)) {
        return "convert_or_replace";
    }
    if (hasStatus(asset, AssetStatus::MissingLicense)) {
        return "add_license_evidence";
    }
    if (hasStatus(asset, AssetStatus::Duplicate) && canArchive) {
        return "archive_duplicate";
    }
    if (canPromote) {
        return "promote";
    }
    if (canAttach) {
        return "attach_to_project";
    }
    if (hasStatus(asset, AssetStatus::Promoted)) {
        return isProjectAttached(asset) ? "project_attached" : "ready";
    }
    if (hasStatus(asset, AssetStatus::Archived)) {
        return "archived";
    }
    return "review";
}

bool hasPreviewPayload(const AssetRecord& asset) {
    return !asset.preview_path.empty() &&
           (asset.preview_kind == "image" || asset.preview_kind == "audio" || asset.preview_kind == "video");
}

nlohmann::json previewStatus(const AssetRecord& asset) {
    if (hasStatus(asset, AssetStatus::MissingFile)) {
        return "missing_file";
    }
    if (hasStatus(asset, AssetStatus::UnsupportedFormat)) {
        return "unsupported_format";
    }
    if (asset.preview_path.empty()) {
        return "missing_preview";
    }
    if (!hasPreviewPayload(asset)) {
        return "unsupported_preview_kind";
    }
    if (asset.preview_kind == "image" && (asset.preview_width <= 0 || asset.preview_height <= 0)) {
        return "thumbnail_pending";
    }
    if (asset.preview_kind == "audio" && asset.waveform_peaks.empty()) {
        return "waveform_pending";
    }
    return "ready";
}

nlohmann::json sequenceMetadata(const AssetRecord& asset) {
    const bool isSequence = asset.media_kind == "image_sequence_collection" || asset.media_kind == "image_sequence";
    return {
        {"visible", isSequence},
        {"frame_count", asset.frame_count},
        {"sequence_count", asset.sequence_count},
        {"representative_sequences",
         asset.representative_sequences.is_array() ? asset.representative_sequences : nlohmann::json::array()},
    };
}

} // namespace

nlohmann::json buildAssetActionRows(const AssetLibrarySnapshot& snapshot) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& asset : snapshot.assets) {
        const auto promoteReason = promoteBlockReason(asset);
        const auto archiveReason = archiveBlockReason(asset);
        const auto attachReason = attachBlockReason(asset);
        const bool canPromote = promoteReason.empty();
        const bool canArchive = archiveReason.empty();
        const bool canAttach = attachReason.empty();
        rows.push_back({
            {"asset_id", asset.asset_id},
            {"path", asset.path},
            {"source_path", asset.source_path},
            {"normalized_path", asset.normalized_path},
            {"preview_path", asset.preview_path},
            {"preview_kind", asset.preview_kind},
            {"preview_width", asset.preview_width},
            {"preview_height", asset.preview_height},
            {"media_kind", asset.media_kind},
            {"category", asset.category},
            {"game_use_category", asset.game_use_category},
            {"pack", asset.pack},
            {"source_bundle_id", asset.source_bundle_id},
            {"package_destination", asset.package_destination},
            {"distribution", asset.distribution},
            {"duplicate_of", asset.duplicate_of},
            {"tags", asset.tags},
            {"game_use_tags", asset.game_use_tags},
            {"used_by", asset.used_by},
            {"sequence", sequenceMetadata(asset)},
            {"statuses", statusList(asset)},
            {"promotion_status", asset.promotion_status},
            {"promoted_path", asset.promoted_path},
            {"license_id", asset.license_id},
            {"include_in_runtime", asset.include_in_runtime},
            {"required_for_release", asset.required_for_release},
            {"release_eligible", asset.release_eligible || asset.provenance.export_eligible},
            {"promotion_diagnostics", asset.promotion_diagnostics},
            {"project_attached", isProjectAttached(asset)},
            {"recommended_action", recommendedAction(asset, canPromote, canArchive, canAttach)},
            {"promote_button",
             {
                 {"visible", true},
                 {"enabled", canPromote},
                 {"action", "promote_asset"},
                 {"disabled_reason", canPromote ? nlohmann::json(nullptr) : nlohmann::json(promoteReason)},
             }},
            {"archive_button",
             {
                 {"visible", true},
                 {"enabled", canArchive},
                 {"action", "archive_asset"},
                 {"disabled_reason", canArchive ? nlohmann::json(nullptr) : nlohmann::json(archiveReason)},
             }},
            {"attach_button",
             {
                 {"visible", true},
                 {"enabled", canAttach},
                 {"action", "attach_project_asset"},
                 {"disabled_reason", canAttach ? nlohmann::json(nullptr) : nlohmann::json(attachReason)},
             }},
        });
    }
    std::sort(rows.begin(), rows.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.value("path", "") < rhs.value("path", ""); });
    return rows;
}

nlohmann::json buildAssetPreviewRows(const AssetLibrarySnapshot& snapshot) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& asset : snapshot.assets) {
        const bool isImage = asset.preview_kind == "image";
        const bool isAudio = asset.preview_kind == "audio";
        const bool isVideo = asset.preview_kind == "video";
        nlohmann::json waveform = nlohmann::json::array();
        for (const auto peak : asset.waveform_peaks) {
            waveform.push_back(peak);
        }
        rows.push_back({
            {"path", asset.path},
            {"normalized_path", asset.normalized_path},
            {"preview_path", asset.preview_path},
            {"preview_kind", asset.preview_kind},
            {"media_kind", asset.media_kind},
            {"category", asset.category},
            {"game_use_category", asset.game_use_category},
            {"pack", asset.pack},
            {"source_bundle_id", asset.source_bundle_id},
            {"game_use_tags", asset.game_use_tags},
            {"status", previewStatus(asset)},
            {"previewable", hasPreviewPayload(asset)},
            {"thumbnail",
             {
                 {"visible", isImage || isVideo},
                 {"ready", (isImage || isVideo) && asset.preview_width > 0 && asset.preview_height > 0},
                 {"width", asset.preview_width},
                 {"height", asset.preview_height},
             }},
            {"waveform",
             {
                 {"visible", isAudio},
                 {"ready", isAudio && !asset.waveform_peaks.empty()},
                 {"peak_count", asset.waveform_peaks.size()},
                 {"duration_ms", asset.duration_ms},
                 {"peaks", waveform},
             }},
            {"sequence", sequenceMetadata(asset)},
        });
    }
    std::sort(rows.begin(), rows.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.value("path", "") < rhs.value("path", ""); });
    return rows;
}

} // namespace urpg::assets
