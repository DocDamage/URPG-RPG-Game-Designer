#pragma once

#include "editor/assets/asset_library_model.h"
#include "engine/core/tools/export_packager.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class AssetLibraryPanel {
  public:
    enum class ImportSourcePickerMode {
        FileOrArchive,
        Folder,
    };

    struct ImportSourcePickerRequest {
        ImportSourcePickerMode mode = ImportSourcePickerMode::FileOrArchive;
        std::filesystem::path library_root;
        std::string session_id;
        std::string license_note;
        std::vector<std::string> external_extractor_command;
    };

    using ImportSourcePicker = std::function<std::optional<std::filesystem::path>(const ImportSourcePickerRequest&)>;

    struct ImportSourcePickerAvailability {
        bool available = false;
        bool path_entry_available = true;
        std::string code;
        std::string message;
    };

    enum class NativeImportSourcePickerPlatform {
        Windows,
        MacOS,
        Linux,
        Unsupported,
    };

    struct ImportWizardStepSnapshot {
        std::string id;
        std::string label;
        std::string state;
        size_t count = 0;
        bool active = false;
        bool complete = false;
        bool available = false;
    };

    struct ImportWizardActionSnapshot {
        std::string id;
        std::string action;
        bool enabled = false;
        bool pending_request = false;
        size_t eligible_count = 0;
        std::string disabled_reason;
    };

    struct ImportWizardRenderSnapshot {
        std::string status = "empty";
        std::string current_step = "add_source";
        bool package_validation_ready = false;
        std::vector<ImportWizardStepSnapshot> steps;
        std::vector<ImportWizardActionSnapshot> actions;
        nlohmann::json extractor_configuration = nlohmann::json::object();
        nlohmann::json pending_request = nullptr;
    };

    AssetLibraryModel& model() { return model_; }
    const AssetLibraryModel& model() const { return model_; }

    static ImportSourcePickerAvailability nativeImportSourcePickerAvailability();
    static std::optional<std::filesystem::path> pickNativeImportSource(const ImportSourcePickerRequest& request);
    static ImportSourcePickerAvailability
    nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform platform,
                                                       bool desktop_portal_available,
                                                       bool desktop_helper_available);
    void setImportSourcePicker(ImportSourcePicker picker);
    void render();
    nlohmann::json requestImportSource(const std::filesystem::path& source, const std::filesystem::path& library_root,
                                       std::string session_id, std::string license_note = {},
                                       std::vector<std::string> external_extractor_command = {},
                                       std::vector<std::string> selected_archive_entries = {});
    nlohmann::json requestImportSourceFromPicker(ImportSourcePickerRequest request);
    nlohmann::json executePendingImportRequest(AssetLibraryModel::ConversionCommandExecutor executor = {});
    nlohmann::json refreshExternalCatalog(AssetLibraryModel::ConversionCommandExecutor executor = {});
    nlohmann::json openSelectedExternalCatalogSource(AssetLibraryModel::ConversionCommandExecutor executor = {});
    nlohmann::json convertSelectedImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                                AssetLibraryModel::ConversionCommandExecutor executor = {});
    nlohmann::json promoteSelectedImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                                std::string license_id, std::string promoted_root,
                                                bool include_in_runtime = true);
    nlohmann::json planPromotedAssetAttachmentToProject(
        std::string path, const std::filesystem::path& project_root,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json confirmPromotedAssetAttachmentToProject(
        std::string path, const std::filesystem::path& project_root, std::string expected_source_revision,
        std::string operation_id,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json planDerivedRevisionAttachmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json confirmDerivedRevisionAttachmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json recoverDerivedAttachmentReference(
        const std::filesystem::path& derived_manifest_path, const std::filesystem::path& project_root);
    nlohmann::json planDerivedTilesetAssignmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json confirmDerivedTilesetAssignmentToProject(
        std::string source_path, const std::filesystem::path& derived_manifest_path,
        const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
        urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
            urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    nlohmann::json createImageCropScaleRevision(
        std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
        int32_t crop_x, int32_t crop_y, int32_t crop_width, int32_t crop_height, int32_t output_width,
        int32_t output_height);
    nlohmann::json createImagePaletteRevision(std::string source_path, const std::filesystem::path& derived_root,
                                              std::string operation_id, std::vector<uint32_t> colors_rgba,
                                              bool dither);
    nlohmann::json createImagePaletteExtractRevision(std::string source_path,
                                                     const std::filesystem::path& derived_root,
                                                     std::string operation_id, int32_t max_colors, bool dither);
    nlohmann::json createAudioTrimFadeGainRevision(
        std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
        uint64_t start_frame, uint64_t end_frame, uint64_t fade_in_frames, uint64_t fade_out_frames,
        int32_t gain_milli_db, int64_t loop_start_frame = -1, int64_t loop_end_frame = -1);
    nlohmann::json inspectAudioTrimFadeGainSource(std::string source_path) const;
    nlohmann::json setPromotedAudioVoiceMetadata(std::string source_path, const std::filesystem::path& library_root,
                                                 std::string locale, std::string take_id,
                                                 std::string muted_alternative_asset_id = {});
    nlohmann::json recoverStagedDerivedRevisions(std::string source_path, const std::filesystem::path& derived_root);
    nlohmann::json removeDerivedRevision(std::string source_path, const std::filesystem::path& derived_root,
                                         std::string derived_revision);
    nlohmann::json createTilesetSliceRevision(std::string source_path, const std::filesystem::path& derived_root,
                                              std::string operation_id, int32_t tile_width, int32_t tile_height,
                                              int32_t margin, int32_t spacing);
    nlohmann::json createAtlasMetadataRevision(std::string source_path, const std::filesystem::path& derived_root,
                                               std::string operation_id, int32_t atlas_width, int32_t atlas_height,
                                               int32_t frame_width, int32_t frame_height);
    nlohmann::json attachSelectedPromotedAssetsToProject(std::vector<std::string> paths,
                                                         const std::filesystem::path& project_root,
                                                         urpg::assets::ProjectAssetAttachmentConflictPolicy policy =
                                                             urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel);
    // Inspection is intentionally read-only: a listed archive is still raw
    // external material until it is reviewed, promoted, and attached.
    nlohmann::json browseArchive(const std::filesystem::path& archive_path);
    nlohmann::json validatePackage(const urpg::tools::ExportConfig& config);
    const AssetLibraryModelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    const ImportWizardRenderSnapshot& lastImportWizardSnapshot() const { return last_import_wizard_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

  private:
    void refreshRenderSnapshotsFromModel();

    AssetLibraryModel model_;
    ImportSourcePicker import_source_picker_{};
    AssetLibraryModelSnapshot last_render_snapshot_{};
    ImportWizardRenderSnapshot last_import_wizard_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
