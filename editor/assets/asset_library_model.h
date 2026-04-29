#pragma once

#include "engine/core/assets/asset_cleanup_planner.h"
#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <map>
#include <string>

namespace urpg::editor {

struct AssetLibraryModelSnapshot {
    std::string status = "empty";
    size_t asset_count = 0;
    size_t catalog_asset_count = 0;
    size_t canonical_asset_count = 0;
    size_t issue_count = 0;
    size_t duplicate_group_count = 0;
    size_t duplicate_asset_count = 0;
    size_t unsupported_count = 0;
    size_t catalog_shard_count = 0;
    size_t referenced_asset_count = 0;
    size_t runtime_ready_count = 0;
    size_t previewable_count = 0;
    size_t promoted_count = 0;
    size_t archived_count = 0;
    size_t filtered_asset_count = 0;
    size_t cleanup_allowed_count = 0;
    size_t cleanup_refused_count = 0;
    bool export_eligible = false;
    bool reports_loaded = false;
    std::string promotion_status;
    nlohmann::json asset_action_rows = nlohmann::json::array();
    nlohmann::json asset_preview_rows = nlohmann::json::array();
    nlohmann::json last_action = nlohmann::json::object();
    nlohmann::json action_history = nlohmann::json::array();
    std::map<std::string, size_t> category_counts;
    std::map<std::string, size_t> kind_counts;
    std::string status_message = "No asset library reports are loaded.";
    std::string error_message;
    std::string remediation = "Run tools/assets/asset_hygiene.py --write-reports to generate asset library reports.";
};

class AssetLibraryModel {
  public:
    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       std::string_view duplicate_csv);
    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       const nlohmann::json& promotion_catalog, std::string_view duplicate_csv);
    bool loadReportsFromDirectory(const std::filesystem::path& reports_root, std::string* error_message = nullptr);
    void addReferencedAsset(std::string path);
    void addUsageReference(std::string path, std::string owner_id);
    urpg::assets::AssetLibraryActionResult promoteAsset(std::string path);
    urpg::assets::AssetLibraryActionResult archiveAsset(std::string path, std::string reason = {});
    void setFilter(urpg::assets::AssetLibraryFilter filter);
    void rebuildCleanupPreview();
    void clear();

    const urpg::assets::AssetLibrary& library() const { return library_; }
    const urpg::assets::AssetCleanupPlan& cleanupPlan() const { return cleanup_plan_; }
    const AssetLibraryModelSnapshot& snapshot() const { return snapshot_; }

  private:
    void refreshSnapshot();

    urpg::assets::AssetLibrary library_;
    urpg::assets::AssetCleanupPlanner cleanup_planner_;
    urpg::assets::AssetCleanupPlan cleanup_plan_;
    urpg::assets::AssetLibraryFilter filter_;
    AssetLibraryModelSnapshot snapshot_{};
    nlohmann::json action_history_ = nlohmann::json::array();
};

} // namespace urpg::editor
