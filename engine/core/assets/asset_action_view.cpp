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

std::string recommendedAction(const AssetRecord& asset, bool canPromote, bool canArchive) {
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
    if (hasStatus(asset, AssetStatus::Promoted)) {
        return "ready";
    }
    if (hasStatus(asset, AssetStatus::Archived)) {
        return "archived";
    }
    return "review";
}

} // namespace

nlohmann::json buildAssetActionRows(const AssetLibrarySnapshot& snapshot) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& asset : snapshot.assets) {
        const auto promoteReason = promoteBlockReason(asset);
        const auto archiveReason = archiveBlockReason(asset);
        const bool canPromote = promoteReason.empty();
        const bool canArchive = archiveReason.empty();
        rows.push_back({
            {"path", asset.path},
            {"source_path", asset.source_path},
            {"normalized_path", asset.normalized_path},
            {"preview_path", asset.preview_path},
            {"preview_kind", asset.preview_kind},
            {"media_kind", asset.media_kind},
            {"category", asset.category},
            {"pack", asset.pack},
            {"duplicate_of", asset.duplicate_of},
            {"tags", asset.tags},
            {"used_by", asset.used_by},
            {"statuses", statusList(asset)},
            {"recommended_action", recommendedAction(asset, canPromote, canArchive)},
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
        });
    }
    std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.value("path", "") < rhs.value("path", "");
    });
    return rows;
}

} // namespace urpg::assets
