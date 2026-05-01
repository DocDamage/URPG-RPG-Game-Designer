#pragma once

#include "editor/assets/asset_library_model.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

class AssetLibraryPanel {
public:
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
        nlohmann::json pending_request = nullptr;
    };

    AssetLibraryModel& model() { return model_; }
    const AssetLibraryModel& model() const { return model_; }

    void render();
    nlohmann::json requestImportSource(const std::filesystem::path& source,
                                       const std::filesystem::path& library_root,
                                       std::string session_id,
                                       std::string license_note = {});
    nlohmann::json promoteSelectedImportRecords(std::string session_id,
                                                std::vector<std::string> asset_ids,
                                                std::string license_id,
                                                std::string promoted_root,
                                                bool include_in_runtime = true);
    nlohmann::json attachSelectedPromotedAssetsToProject(std::vector<std::string> paths,
                                                         const std::filesystem::path& project_root);
    const AssetLibraryModelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    const ImportWizardRenderSnapshot& lastImportWizardSnapshot() const { return last_import_wizard_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

private:
    void refreshRenderSnapshotsFromModel();

    AssetLibraryModel model_;
    AssetLibraryModelSnapshot last_render_snapshot_{};
    ImportWizardRenderSnapshot last_import_wizard_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
