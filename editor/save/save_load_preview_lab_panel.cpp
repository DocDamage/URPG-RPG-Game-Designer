#include "editor/save/save_load_preview_lab_panel.h"

#include <utility>

namespace urpg::editor {

void SaveLoadPreviewLabPanel::loadDocument(urpg::save::SaveLoadPreviewLabDocument document,
                                           std::filesystem::path workspace_root) {
    document_ = std::move(document);
    workspace_root_ = std::move(workspace_root);
    loaded_ = true;
    refreshPreview();
}

void SaveLoadPreviewLabPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a save/load preview lab before rendering this panel.";
        return;
    }
    refreshPreview();
}

void SaveLoadPreviewLabPanel::refreshPreview() {
    result_ = urpg::save::RunSaveLoadPreviewLab(document_, workspace_root_);
    snapshot_.disabled = false;
    snapshot_.lab_id = document_.id;
    snapshot_.saved_primary = result_.saved_primary;
    snapshot_.loaded_ok = result_.loaded_ok;
    snapshot_.loaded_from_recovery = result_.loaded_from_recovery;
    snapshot_.boot_safe_mode = result_.boot_safe_mode;
    snapshot_.recovery_tier = recoveryTierLabel(result_.recovery_tier);
    snapshot_.payload_matches_expected = result_.payload_matches_expected;
    snapshot_.loaded_slot_id = result_.loaded_meta.slot_id;
    snapshot_.loaded_map_display_name = result_.loaded_meta.map_display_name;
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Save/load preview lab is ready." : "Save/load preview lab has diagnostics.";
}

std::string SaveLoadPreviewLabPanel::recoveryTierLabel(urpg::SaveRecoveryTier tier) {
    switch (tier) {
    case urpg::SaveRecoveryTier::None:
        return "none";
    case urpg::SaveRecoveryTier::Level1Autosave:
        return "level1_autosave";
    case urpg::SaveRecoveryTier::Level2MetadataVariables:
        return "level2_metadata_variables";
    case urpg::SaveRecoveryTier::Level3SafeSkeleton:
        return "level3_safe_skeleton";
    }
    return "none";
}

} // namespace urpg::editor
