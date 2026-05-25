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

nlohmann::json assetReadinessDiagnostics(const AssetRecord& asset) {
    nlohmann::json rows = nlohmann::json::array();
    const auto add = [&](const std::string& code,
                         const std::string& severity,
                         const std::string& message,
                         const std::string& target) {
        rows.push_back({{"code", code}, {"severity", severity}, {"message", message}, {"target", target}});
    };

    if (asset.path.empty() || hasStatus(asset, AssetStatus::MissingFile)) {
        add("asset_file_missing", "error", "Asset source file is missing.", asset.path);
    }
    if (hasStatus(asset, AssetStatus::MissingLicense) || asset.license_id.empty()) {
        add("license_evidence_missing", asset.required_for_release ? "error" : "warning",
            "Asset needs license evidence before release packaging.", asset.asset_id);
    }
    if (asset.include_in_runtime && asset.promoted_path.empty()) {
        add("runtime_payload_missing", "error", "Runtime packageable asset needs a promoted payload path.",
            asset.asset_id);
    }
    if (asset.preview_kind == "image" && (asset.preview_path.empty() || asset.preview_width <= 0 ||
                                           asset.preview_height <= 0)) {
        add("thumbnail_preview_missing", asset.required_for_release ? "error" : "warning",
            "Image asset needs thumbnail path and dimensions for WYSIWYG preview.", asset.asset_id);
    }
    if (asset.preview_kind == "audio" && asset.waveform_peaks.empty()) {
        add("waveform_preview_missing", asset.required_for_release ? "error" : "warning",
            "Audio asset needs waveform peaks for WYSIWYG preview.", asset.asset_id);
    }
    if ((asset.preview_kind == "video" || asset.media_kind == "image_sequence" ||
         asset.media_kind == "image_sequence_collection") &&
        (asset.preview_path.empty() || asset.preview_width <= 0 || asset.preview_height <= 0)) {
        add("sequence_preview_missing", asset.required_for_release ? "error" : "warning",
            "Sequence/video asset needs representative preview metadata.", asset.asset_id);
    }
    for (const auto& diagnostic : asset.promotion_diagnostics) {
        add(std::string("promotion_manifest_") + diagnostic, "warning", "Promotion manifest reported " + diagnostic + ".",
            asset.asset_id);
    }
    return rows;
}

nlohmann::json readinessSummary(const AssetRecord& asset) {
    const auto diagnostics = assetReadinessDiagnostics(asset);
    std::size_t errorCount = 0;
    std::size_t warningCount = 0;
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.value("severity", "") == "error") {
            ++errorCount;
        } else if (diagnostic.value("severity", "") == "warning") {
            ++warningCount;
        }
    }
    return {{"package_ready", errorCount == 0 && asset.include_in_runtime && !asset.promoted_path.empty()},
            {"preview_ready", previewStatus(asset) == "ready"},
            {"release_ready", errorCount == 0 && warningCount == 0 && asset.release_eligible},
            {"error_count", errorCount},
            {"warning_count", warningCount},
            {"diagnostics", diagnostics}};
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
            {"readiness", readinessSummary(asset)},
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
            {"readiness", readinessSummary(asset)},
        });
    }
    std::sort(rows.begin(), rows.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.value("path", "") < rhs.value("path", ""); });
    return rows;
}

} // namespace urpg::assets
