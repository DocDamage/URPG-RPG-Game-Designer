#include "editor/assets/asset_library_model.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace urpg::editor {

void AssetLibraryModel::ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                                      std::string_view duplicate_csv) {
    ingestReports(hygiene_summary, intake_report, nlohmann::json::object(), duplicate_csv);
}

void AssetLibraryModel::ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                                      const nlohmann::json& promotion_catalog, std::string_view duplicate_csv) {
    library_.clear();
    library_.ingestHygieneSummary(hygiene_summary);
    library_.ingestIntakeReport(intake_report);
    library_.ingestPromotionCatalog(promotion_catalog);
    library_.ingestDuplicateCsv(duplicate_csv);
    library_.detectCaseCollisions();
    rebuildCleanupPreview();
    snapshot_.reports_loaded = true;
    snapshot_.status = "ready";
    snapshot_.status_message = "";
    snapshot_.error_message = "";
}

namespace {

void ingestCatalogWithShards(urpg::assets::AssetLibrary& library, const std::filesystem::path& reports_root,
                             const nlohmann::json& promotion_catalog) {
    library.ingestPromotionCatalog(promotion_catalog);
    const auto shards = promotion_catalog.find("shards");
    if (shards == promotion_catalog.end() || !shards->is_array()) {
        return;
    }
    const auto repo_root = reports_root.parent_path();
    for (const auto& shard : *shards) {
        if (!shard.is_object()) {
            continue;
        }
        const auto path_it = shard.find("path");
        if (path_it == shard.end() || !path_it->is_string()) {
            continue;
        }
        auto shard_path = std::filesystem::path(path_it->get<std::string>());
        if (shard_path.is_relative()) {
            const auto relative = shard_path;
            const auto repo_relative = repo_root.parent_path() / relative;
            const auto imports_relative = repo_root / relative;
            const auto reports_relative = reports_root / relative;
            if (std::filesystem::is_regular_file(reports_relative)) {
                shard_path = reports_relative;
            } else if (std::filesystem::is_regular_file(repo_relative)) {
                shard_path = repo_relative;
            } else {
                shard_path = imports_relative;
            }
        }
        if (!std::filesystem::is_regular_file(shard_path)) {
            continue;
        }
        std::ifstream shard_stream(shard_path);
        library.ingestPromotionCatalog(nlohmann::json::parse(shard_stream));
    }
}

} // namespace

bool AssetLibraryModel::loadReportsFromDirectory(const std::filesystem::path& reports_root,
                                                 std::string* error_message) {
    const auto hygiene_path = reports_root / "asset_hygiene_summary.json";
    const auto duplicates_path = reports_root / "asset_hygiene_duplicates.csv";
    const auto intake_path = reports_root / "asset_intake" / "source_capture_status.json";
    const auto promotion_catalog_path = reports_root / "asset_intake" / "urpg_stuff_promotion_catalog.json";

    if (!std::filesystem::is_regular_file(hygiene_path) || !std::filesystem::is_regular_file(duplicates_path) ||
        !std::filesystem::is_regular_file(intake_path)) {
        clear();
        snapshot_.status = "empty";
        snapshot_.status_message = "Asset library reports are missing.";
        snapshot_.error_message = "Missing required reports under " + reports_root.string();
        if (error_message != nullptr) {
            *error_message = "asset library reports are missing";
        }
        return false;
    }

    try {
        std::ifstream hygiene_stream(hygiene_path);
        std::ifstream intake_stream(intake_path);
        std::ifstream duplicate_stream(duplicates_path);
        std::stringstream duplicate_buffer;
        duplicate_buffer << duplicate_stream.rdbuf();

        const auto hygiene_summary = nlohmann::json::parse(hygiene_stream);
        const auto intake_report = nlohmann::json::parse(intake_stream);
        nlohmann::json promotion_catalog = nlohmann::json::object();
        if (std::filesystem::is_regular_file(promotion_catalog_path)) {
            std::ifstream promotion_stream(promotion_catalog_path);
            promotion_catalog = nlohmann::json::parse(promotion_stream);
        }
        library_.clear();
        library_.ingestHygieneSummary(hygiene_summary);
        library_.ingestIntakeReport(intake_report);
        ingestCatalogWithShards(library_, reports_root, promotion_catalog);
        library_.ingestDuplicateCsv(duplicate_buffer.str());
        library_.detectCaseCollisions();
        rebuildCleanupPreview();
        snapshot_.reports_loaded = true;
        snapshot_.status = "ready";
        snapshot_.status_message = "";
        snapshot_.error_message = "";
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        clear();
        snapshot_.status = "error";
        snapshot_.status_message = "Asset library reports could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }

    return true;
}

void AssetLibraryModel::addReferencedAsset(std::string path) {
    library_.addReferencedAsset(std::move(path));
    rebuildCleanupPreview();
}

void AssetLibraryModel::addUsageReference(std::string path, std::string owner_id) {
    library_.addUsageReference(std::move(path), std::move(owner_id));
    rebuildCleanupPreview();
}

void AssetLibraryModel::setFilter(urpg::assets::AssetLibraryFilter filter) {
    filter_ = std::move(filter);
    refreshSnapshot();
}

void AssetLibraryModel::rebuildCleanupPreview() {
    cleanup_plan_ = cleanup_planner_.buildDuplicateCleanupPlan(library_);
    refreshSnapshot();
}

void AssetLibraryModel::clear() {
    library_.clear();
    cleanup_plan_ = {};
    snapshot_ = {};
    snapshot_.status = "empty";
    snapshot_.reports_loaded = false;
    snapshot_.status_message = "No asset library reports are loaded.";
    snapshot_.remediation = "Run tools/assets/asset_hygiene.py --write-reports to generate asset library reports.";
}

void AssetLibraryModel::refreshSnapshot() {
    const auto& asset_snapshot = library_.snapshot();
    snapshot_.asset_count = asset_snapshot.assets.size();
    snapshot_.catalog_asset_count = asset_snapshot.catalog_asset_count;
    snapshot_.canonical_asset_count = asset_snapshot.canonical_asset_count;
    snapshot_.duplicate_group_count = asset_snapshot.duplicate_groups.size();
    if (snapshot_.duplicate_group_count == 0) {
        snapshot_.duplicate_group_count = asset_snapshot.duplicate_group_count;
    }
    snapshot_.duplicate_asset_count = asset_snapshot.duplicate_asset_count;
    snapshot_.unsupported_count = asset_snapshot.unsupported_count;
    snapshot_.catalog_shard_count = asset_snapshot.catalog_shard_count;
    snapshot_.referenced_asset_count = asset_snapshot.referenced_asset_count;
    snapshot_.runtime_ready_count = asset_snapshot.runtime_ready_count;
    snapshot_.previewable_count = asset_snapshot.previewable_count;
    snapshot_.filtered_asset_count = library_.filterAssets(filter_).size();
    snapshot_.cleanup_allowed_count = cleanup_plan_.allowed_count;
    snapshot_.cleanup_refused_count = cleanup_plan_.refused_count;
    snapshot_.export_eligible = asset_snapshot.export_eligible;
    snapshot_.promotion_status = asset_snapshot.promotion_status;
    snapshot_.category_counts = asset_snapshot.category_counts;
    snapshot_.kind_counts = asset_snapshot.kind_counts;
    snapshot_.reports_loaded = asset_snapshot.assets.size() > 0 || asset_snapshot.duplicate_groups.size() > 0 ||
                               asset_snapshot.catalog_asset_count > 0 || asset_snapshot.catalog_shard_count > 0 ||
                               cleanup_plan_.allowed_count > 0 || cleanup_plan_.refused_count > 0;
    snapshot_.status = snapshot_.reports_loaded ? "ready" : "empty";
    snapshot_.status_message = snapshot_.reports_loaded ? "" : "No asset library reports are loaded.";
    if (snapshot_.reports_loaded) {
        snapshot_.error_message = "";
    }
    snapshot_.issue_count = 0;
    for (const auto& asset : asset_snapshot.assets) {
        if (!(asset.statuses.size() == 1 && asset.statuses.contains(urpg::assets::AssetStatus::Usable))) {
            ++snapshot_.issue_count;
        }
    }
}

} // namespace urpg::editor
