#pragma once

#include "engine/core/assets/asset_cleanup_planner.h"
#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace urpg::editor {

struct AssetLibraryModelSnapshot {
    size_t asset_count = 0;
    size_t issue_count = 0;
    size_t duplicate_group_count = 0;
    size_t cleanup_allowed_count = 0;
    size_t cleanup_refused_count = 0;
    bool reports_loaded = false;
    std::string status_message = "No asset library reports are loaded.";
};

class AssetLibraryModel {
  public:
    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       std::string_view duplicate_csv);
    bool loadReportsFromDirectory(const std::filesystem::path& reports_root, std::string* error_message = nullptr);
    void addReferencedAsset(std::string path);
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
    AssetLibraryModelSnapshot snapshot_{};
};

} // namespace urpg::editor
