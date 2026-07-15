#pragma once

#include "engine/core/assets/asset_cleanup_planner.h"
#include "engine/core/assets/archive_catalog.h"
#include "engine/core/assets/asset_import_session.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/assets/local_asset_catalog.h"
#include "engine/core/assets/project_asset_attachment_service.h"
#include "engine/core/assets/asset_transform_revision_service.h"
#include "engine/core/settings/app_settings_store.h"

#include <nlohmann/json.hpp>

#include <cstdint>
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
    size_t external_catalog_asset_count = 0;
    size_t external_catalog_hash_pending_count = 0;
    size_t external_catalog_archive_count = 0;
    size_t filtered_asset_count = 0;
    size_t cleanup_allowed_count = 0;
    size_t cleanup_refused_count = 0;
    size_t favorite_asset_count = 0;
    size_t asset_collection_count = 0;
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
    nlohmann::json virtual_catalog = nlohmann::json::object();
    nlohmann::json external_catalog = nlohmann::json::object();
    nlohmann::json archive_browser = nlohmann::json::object();
    nlohmann::json last_action = nlohmann::json::object();
    nlohmann::json action_history = nlohmann::json::array();
    nlohmann::json user_curation = nlohmann::json::object();
    std::map<std::string, size_t> category_counts;
    std::map<std::string, size_t> game_use_category_counts;
    std::map<std::string, size_t> game_use_tag_counts;
    std::map<std::string, size_t> kind_counts;
    std::map<std::string, size_t> source_bundle_counts;
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

    AssetLibraryModel();
    static ConversionCommandResult runConversionCommand(const ConversionCommand& command);
    void setImportToolCommand(std::vector<std::string> command_prefix);

    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       std::string_view duplicate_csv);
    void ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                       const nlohmann::json& promotion_catalog, std::string_view duplicate_csv);
    void ingestPromotionManifest(const urpg::assets::AssetPromotionManifest& manifest);
    nlohmann::json requestImportSource(const std::filesystem::path& source, const std::filesystem::path& library_root,
                                       std::string session_id, std::string license_note = {},
                                       std::vector<std::string> external_extractor_command = {},
                                       std::vector<std::string> selected_archive_entries = {});
    // Runs the explicitly requested importer without a shell, then loads the
    // session manifest it produced. Keeping this handoff here means the
    // editor never has to infer an import result from a console command.
    nlohmann::json executePendingImportRequest(ConversionCommandExecutor executor = {});
    void ingestImportSession(urpg::assets::AssetImportSession session);
    void clearImportSessions();
    bool loadImportSessionManifest(const std::filesystem::path& manifest_path, std::string* error_message = nullptr);
    bool loadImportSessionsFromLibraryRoot(const std::filesystem::path& library_root,
                                           std::string* error_message = nullptr);
    bool loadPromotedAssetsFromLibraryRoot(const std::filesystem::path& library_root,
                                           std::string* error_message = nullptr);
    urpg::assets::AssetLibraryActionResult promoteImportRecord(std::string session_id, std::string asset_id,
                                                               std::string license_id, std::string promoted_root,
                                                               bool include_in_runtime = true);
    nlohmann::json runImportRecordConversion(std::string session_id, std::string asset_id,
                                             ConversionCommandExecutor executor = {});
    nlohmann::json runImportRecordConversions(std::string session_id, std::vector<std::string> asset_ids,
                                              ConversionCommandExecutor executor = {});
    // Stores an explicit loose-spritesheet grid in the governed import-session
    // manifest. The external source file is never modified.
    nlohmann::json setImportRecordSpriteSheetSlice(std::string session_id, std::string asset_id,
                                                   int32_t frame_width, int32_t frame_height, int32_t rows,
                                                   int32_t columns, std::string direction, bool loop,
                                                   float frame_duration);
    nlohmann::json promoteImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                        std::string license_id, std::string promoted_root,
                                        bool include_in_runtime = true);
    urpg::assets::AssetLibraryActionResult
    promoteImportRecordToGlobalLibrary(std::string session_id, std::string asset_id, std::string license_id,
                                       const std::filesystem::path& promoted_root);
    nlohmann::json promoteImportRecordsToGlobalLibrary(std::string session_id, std::vector<std::string> asset_ids,
                                                       std::string license_id,
                                                       const std::filesystem::path& promoted_root);
    bool loadReportsFromDirectory(const std::filesystem::path& reports_root, std::string* error_message = nullptr);
    void setDuplicateCsvDetailLimitBytes(std::uintmax_t limit_bytes);
    void setPromotionCatalogDetailLimitBytes(std::uintmax_t limit_bytes);
    bool loadExternalCatalog(const std::filesystem::path& catalog_directory, std::string* error_message = nullptr);
    void setExternalCatalogQuery(urpg::assets::LocalAssetCatalogQuery query);
    void selectExternalCatalogAsset(std::string asset_id);
    nlohmann::json refreshExternalCatalog(ConversionCommandExecutor executor = {});
    nlohmann::json openSelectedExternalCatalogSource(ConversionCommandExecutor executor = {});
    nlohmann::json browseArchive(const std::filesystem::path& archive_path);
    const urpg::assets::LocalAssetCatalog& externalCatalog() const { return external_catalog_; }
    bool loadAssetBundleManifestsFromDirectory(const std::filesystem::path& bundle_root,
                                               std::string* error_message = nullptr);
    void addReferencedAsset(std::string path);
    void addUsageReference(std::string path, std::string owner_id);
    urpg::assets::AssetLibraryActionResult promoteAsset(std::string path);
    urpg::assets::AssetLibraryActionResult archiveAsset(std::string path, std::string reason = {});
    // A plan is creator-visible and contains the revision that must be supplied
    // unchanged when confirming the attachment.
    nlohmann::json planPromotedAssetAttachmentToProject(
        std::string path, const std::filesystem::path& project_root,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    urpg::assets::AssetLibraryActionResult confirmPromotedAssetAttachmentToProject(
        std::string path, const std::filesystem::path& project_root, std::string expected_source_revision,
        std::string operation_id,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json planDerivedRevisionAttachmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    urpg::assets::AssetLibraryActionResult confirmDerivedRevisionAttachmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json createImageCropScaleRevision(
        std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
        int32_t crop_x, int32_t crop_y, int32_t crop_width, int32_t crop_height, int32_t output_width,
        int32_t output_height);
    nlohmann::json createImagePaletteRevision(std::string source_path, const std::filesystem::path& derived_root,
                                              std::string operation_id, std::vector<uint32_t> colors_rgba);
    nlohmann::json createAudioTrimFadeGainRevision(
        std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
        uint64_t start_frame, uint64_t end_frame, uint64_t fade_in_frames, uint64_t fade_out_frames,
        int32_t gain_milli_db, int64_t loop_start_frame = -1, int64_t loop_end_frame = -1);
    nlohmann::json createTilesetSliceRevision(std::string source_path, const std::filesystem::path& derived_root,
                                              std::string operation_id, int32_t tile_width, int32_t tile_height,
                                              int32_t margin, int32_t spacing);
    nlohmann::json createAtlasMetadataRevision(std::string source_path, const std::filesystem::path& derived_root,
                                               std::string operation_id, int32_t atlas_width, int32_t atlas_height,
                                               int32_t frame_width, int32_t frame_height);
    // Compatibility shortcut. New creator-facing callers should show the plan
    // and invoke the confirmation overload with its revision and operation ID.
    urpg::assets::AssetLibraryActionResult attachPromotedAssetToProject(std::string path,
                                                                        const std::filesystem::path& project_root,
                                                                        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
                                                                            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json attachPromotedAssetsToProject(std::vector<std::string> paths,
                                                 const std::filesystem::path& project_root,
                                                 urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
                                                     urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    bool loadProjectAssetAttachments(const std::filesystem::path& project_root, std::string* error_message = nullptr);
    void setFilter(urpg::assets::AssetLibraryFilter filter);
    bool applyQuickFilter(std::string_view filter_id);
    void applyUserAssetCuration(const urpg::settings::EditorSettings& settings);
    void writeUserAssetCuration(urpg::settings::EditorSettings* settings) const;
    bool isAssetFavorite(std::string_view path) const;
    bool isAssetInCollection(std::string_view collection_id, std::string_view path) const;
    bool setAssetFavorite(std::string_view path, bool favorite);
    bool createAssetCollection(std::string id, std::string label);
    bool setAssetCollectionMembership(std::string_view collection_id, std::string_view path, bool included);
    void rebuildCleanupPreview();
    void clear();

    const urpg::assets::AssetLibrary& library() const { return library_; }
    const urpg::assets::AssetCleanupPlan& cleanupPlan() const { return cleanup_plan_; }
    const AssetLibraryModelSnapshot& snapshot() const { return snapshot_; }

  private:
    bool persistImportSession(const urpg::assets::AssetImportSession& session, std::string* error_message);
    void refreshSnapshot();
    void refreshExternalCatalogSnapshot();
    std::string curationKeyForPath(std::string_view path) const;

    urpg::assets::AssetLibrary library_;
    urpg::assets::AssetCleanupPlanner cleanup_planner_;
    urpg::assets::AssetCleanupPlan cleanup_plan_;
    urpg::assets::LocalAssetCatalog external_catalog_;
    urpg::assets::LocalAssetCatalogQuery external_catalog_query_;
    std::filesystem::path external_catalog_directory_;
    std::string selected_external_catalog_asset_id_;
    std::vector<std::string> external_catalog_diagnostics_;
    nlohmann::json archive_browser_ = nlohmann::json::object();
    std::filesystem::path cached_archive_path_;
    uintmax_t cached_archive_size_ = 0;
    std::filesystem::file_time_type cached_archive_write_time_{};
    std::vector<urpg::assets::AssetImportSession> import_sessions_;
    std::map<std::string, std::filesystem::path> import_session_manifest_paths_;
    urpg::assets::AssetLibraryFilter filter_;
    AssetLibraryModelSnapshot snapshot_{};
    nlohmann::json action_history_ = nlohmann::json::array();
    nlohmann::json pending_import_request_ = nlohmann::json::object();
    std::vector<std::string> import_tool_command_ = {"python", "tools/assets/global_asset_import.py"};
    std::vector<std::string> favorite_asset_keys_;
    std::vector<urpg::settings::AssetLibraryCollectionSettings> asset_collections_;
    std::uintmax_t duplicate_csv_detail_limit_bytes_ = 8ull * 1024ull * 1024ull;
    std::uintmax_t promotion_catalog_detail_limit_bytes_ = 4ull * 1024ull * 1024ull;
};

} // namespace urpg::editor
