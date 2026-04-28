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

void SaveLoadPreviewLabPanel::setSlotId(int32_t slot_id) {
    document_.slot_id = slot_id;
    if (!document_.metadata_payload.is_object()) {
        document_.metadata_payload = nlohmann::json::object();
    }
    document_.metadata_payload["_slot_id"] = slot_id;
    if (loaded_) {
        refreshPreview();
    }
}

void SaveLoadPreviewLabPanel::setPrimaryPayloadField(std::string key, nlohmann::json value) {
    if (!document_.primary_payload.is_object()) {
        document_.primary_payload = nlohmann::json::object();
    }
    document_.primary_payload[std::move(key)] = std::move(value);
    if (loaded_) {
        refreshPreview();
    }
}

void SaveLoadPreviewLabPanel::setVariablePayloadField(std::string key, nlohmann::json value) {
    if (!document_.variables_payload.is_object()) {
        document_.variables_payload = nlohmann::json::object();
    }
    document_.variables_payload[std::move(key)] = std::move(value);
    if (loaded_) {
        refreshPreview();
    }
}

void SaveLoadPreviewLabPanel::setCorruptPrimary(bool corrupt) {
    document_.corrupt_primary = corrupt;
    if (loaded_) {
        refreshPreview();
    }
}

void SaveLoadPreviewLabPanel::setForceSafeMode(bool force_safe_mode) {
    document_.force_safe_mode = force_safe_mode;
    if (loaded_) {
        refreshPreview();
    }
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
    snapshot_.variables_payload_matches = result_.variables_payload_matches;
    snapshot_.payload_diff_count = result_.payload_diff_count;
    snapshot_.runtime_trace_count = result_.runtime_trace.size();
    snapshot_.loaded_slot_id = result_.loaded_meta.slot_id;
    snapshot_.loaded_map_display_name = result_.loaded_meta.map_display_name;
    snapshot_.diagnostic_count = result_.diagnostics.size();
    float passed_checks = 0.0f;
    passed_checks += snapshot_.saved_primary ? 1.0f : 0.0f;
    passed_checks += snapshot_.loaded_ok ? 1.0f : 0.0f;
    passed_checks += snapshot_.payload_matches_expected ? 1.0f : 0.0f;
    passed_checks += snapshot_.variables_payload_matches ? 1.0f : 0.0f;
    snapshot_.integrity_score = passed_checks / 4.0f;
    snapshot_.integrity_summary = snapshot_.loaded_from_recovery
        ? "Loaded through " + snapshot_.recovery_tier
        : (snapshot_.loaded_ok ? "Primary save round-trip is clean." : "Save/load round-trip failed.");
    if (snapshot_.loaded_from_recovery) {
        snapshot_.ux_focus_lane = "recovery";
        snapshot_.primary_action = "Inspect recovery behavior and safe-mode boot state.";
    } else if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Resolve save lab diagnostics before accepting the slot.";
    } else if (!snapshot_.payload_matches_expected || !snapshot_.variables_payload_matches) {
        snapshot_.ux_focus_lane = "diff";
        snapshot_.primary_action = "Review payload diffs and variable preservation.";
    } else {
        snapshot_.ux_focus_lane = "roundtrip";
        snapshot_.primary_action = "Use this save/load trace as the expected runtime contract.";
    }
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
