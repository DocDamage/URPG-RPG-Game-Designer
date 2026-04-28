#pragma once

#include "engine/core/save/save_load_preview_lab.h"

#include <filesystem>
#include <string>

namespace urpg::editor {

struct SaveLoadPreviewLabPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string lab_id;
    bool saved_primary = false;
    bool loaded_ok = false;
    bool loaded_from_recovery = false;
    bool boot_safe_mode = false;
    std::string recovery_tier;
    bool payload_matches_expected = false;
    int32_t loaded_slot_id = 0;
    std::string loaded_map_display_name;
    size_t diagnostic_count = 0;
    std::string saved_project_json;
    std::string status_message = "Load a save/load preview lab before rendering this panel.";
};

class SaveLoadPreviewLabPanel {
public:
    void loadDocument(urpg::save::SaveLoadPreviewLabDocument document, std::filesystem::path workspace_root);
    void render();

    const SaveLoadPreviewLabPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::save::SaveLoadPreviewLabResult& result() const { return result_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();
    static std::string recoveryTierLabel(urpg::SaveRecoveryTier tier);

    urpg::save::SaveLoadPreviewLabDocument document_;
    std::filesystem::path workspace_root_;
    urpg::save::SaveLoadPreviewLabResult result_;
    SaveLoadPreviewLabPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
