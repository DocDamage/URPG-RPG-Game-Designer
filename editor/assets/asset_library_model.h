#pragma once

#include "engine/core/assets/asset_cleanup_planner.h"
#include "engine/core/assets/asset_import_session.h"
#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

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
    size_t sequence_asset_count = 0;
    size_t sequence_frame_count = 0;
    size_t sequence_clip_count = 0;
    size_t promoted_count = 0;
    size_t archived_count = 0;
    size_t project_attached_count = 0;
    size_t project_attachable_count = 0;
    size_t import_session_count = 0;
    size_t import_review_row_count = 0;
    size_t import_ready_count = 0;
    size_t import_needs_conversion_count = 0;
    size_t import_duplicate_count = 0;
    size_t import_missing_license_count = 0;
    size_t import_unsupported_count = 0;
    size_t import_source_only_count = 0;
    size_t import_error_count = 0;
    size_t filtered_asset_count = 0;
    size_t cleanup_allowed_count = 0;
    size_t cleanup_refused_count = 0;
    bool export_eligible = false;
    bool reports_loaded = false;
    std::string promotion_status;
    nlohmann::json filter_controls = nlohmann::json::object();
    nlohmann::json asset_action_rows = nlohmann::json::array();
    nlohmann::json asset_preview_rows = nlohmann::json::array();
    nlohmann::json project_asset_picker_rows = nlohmann::json::array();
    nlohmann::json import_session_rows = nlohmann::json::array();
    nlohmann::json import_review_rows = nlohmann::json::array();
    nlohmann::json import_wizard = nlohmann::json::object();
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
    struct ConversionCommand {
        std::filesystem::path working_directory;
        std::filesystem::path output_path;
        std::vector<std::string> arguments;
    };

    struct ConversionCommandResult {
        int exit_code = 0;
        std::string stdout_text;
        std::string stderr_text;
    };

    using ConversionCommandExecutor = std::function<ConversionCommandResult(const ConversionCommand&)>;

    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       std::string_view duplicate_csv);
    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       const nlohmann::json& promotion_catalog, std::string_view duplicate_csv);
    void ingestPromotionManifest(const urpg::assets::AssetPromotionManifest& manifest);
    nlohmann::json requestImportSource(const std::filesystem::path& source,
                                       const std::filesystem::path& library_root,
                                       std::string session_id,
                                       std::string license_note = {});
    void ingestImportSession(urpg::assets::AssetImportSession session);
    void clearImportSessions();
    bool loadImportSessionManifest(const std::filesystem::path& manifest_path, std::string* error_message = nullptr);
    bool loadImportSessionsFromLibraryRoot(const std::filesystem::path& library_root, std::string* error_message = nullptr);
    bool loadPromotedAssetsFromLibraryRoot(const std::filesystem::path& library_root, std::string* error_message = nullptr);
    urpg::assets::AssetLibraryActionResult promoteImportRecord(std::string session_id, std::string asset_id,
                                                               std::string license_id, std::string promoted_root,
                                                               bool include_in_runtime = true);
    nlohmann::json runImportRecordConversion(std::string session_id, std::string asset_id,
                                             ConversionCommandExecutor executor = {});
    nlohmann::json runImportRecordConversions(std::string session_id, std::vector<std::string> asset_ids,
                                              ConversionCommandExecutor executor = {});
    nlohmann::json promoteImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                        std::string license_id, std::string promoted_root,
                                        bool include_in_runtime = true);
    urpg::assets::AssetLibraryActionResult promoteImportRecordToGlobalLibrary(std::string session_id,
                                                                              std::string asset_id,
                                                                              std::string license_id,
                                                                              const std::filesystem::path& promoted_root);
    nlohmann::json promoteImportRecordsToGlobalLibrary(std::string session_id, std::vector<std::string> asset_ids,
                                                       std::string license_id,
                                                       const std::filesystem::path& promoted_root);
    bool loadReportsFromDirectory(const std::filesystem::path& reports_root, std::string* error_message = nullptr);
    void addReferencedAsset(std::string path);
    void addUsageReference(std::string path, std::string owner_id);
    urpg::assets::AssetLibraryActionResult promoteAsset(std::string path);
    urpg::assets::AssetLibraryActionResult archiveAsset(std::string path, std::string reason = {});
    urpg::assets::AssetLibraryActionResult attachPromotedAssetToProject(std::string path,
                                                                         const std::filesystem::path& project_root);
    nlohmann::json attachPromotedAssetsToProject(std::vector<std::string> paths,
                                                 const std::filesystem::path& project_root);
    bool loadProjectAssetAttachments(const std::filesystem::path& project_root, std::string* error_message = nullptr);
    void setFilter(urpg::assets::AssetLibraryFilter filter);
    bool applyQuickFilter(std::string_view filter_id);
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
    std::vector<urpg::assets::AssetImportSession> import_sessions_;
    urpg::assets::AssetLibraryFilter filter_;
    AssetLibraryModelSnapshot snapshot_{};
    nlohmann::json action_history_ = nlohmann::json::array();
    nlohmann::json pending_import_request_ = nlohmann::json::object();
};

} // namespace urpg::editor
