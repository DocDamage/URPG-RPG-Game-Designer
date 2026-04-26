#include "editor/assets/asset_library_model.h"

#include <fstream>
#include <sstream>

namespace urpg::editor {

void AssetLibraryModel::ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                                      std::string_view duplicate_csv) {
    library_.clear();
    library_.ingestHygieneSummary(hygiene_summary);
    library_.ingestIntakeReport(intake_report);
    library_.ingestDuplicateCsv(duplicate_csv);
    library_.detectCaseCollisions();
    rebuildCleanupPreview();
    snapshot_.reports_loaded = true;
    snapshot_.status = "ready";
    snapshot_.status_message = "";
    snapshot_.error_message = "";
}

bool AssetLibraryModel::loadReportsFromDirectory(const std::filesystem::path& reports_root,
                                                 std::string* error_message) {
    const auto hygiene_path = reports_root / "asset_hygiene_summary.json";
    const auto duplicates_path = reports_root / "asset_hygiene_duplicates.csv";
    const auto intake_path = reports_root / "asset_intake" / "source_capture_status.json";

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
        ingestReports(hygiene_summary, intake_report, duplicate_buffer.str());
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
    snapshot_.duplicate_group_count = asset_snapshot.duplicate_groups.size();
    snapshot_.cleanup_allowed_count = cleanup_plan_.allowed_count;
    snapshot_.cleanup_refused_count = cleanup_plan_.refused_count;
    snapshot_.reports_loaded = asset_snapshot.assets.size() > 0 || asset_snapshot.duplicate_groups.size() > 0 ||
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
