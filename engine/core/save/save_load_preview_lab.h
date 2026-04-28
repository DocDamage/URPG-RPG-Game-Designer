#pragma once

#include "engine/core/save/save_runtime.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::save {

struct SaveLoadPreviewDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct SaveLoadPreviewLabDocument {
    std::string id;
    int32_t slot_id = 1;
    nlohmann::json primary_payload = nlohmann::json::object();
    nlohmann::json autosave_payload = nlohmann::json::object();
    nlohmann::json metadata_payload = nlohmann::json::object();
    nlohmann::json variables_payload = nlohmann::json::object();
    bool corrupt_primary = false;
    bool force_safe_mode = false;
    std::string safe_mode_fallback_map = "Safe Mode - Origin";

    std::vector<SaveLoadPreviewDiagnostic> validate() const;
    nlohmann::json toJson() const;

    static SaveLoadPreviewLabDocument fromJson(const nlohmann::json& json);
};

struct SaveLoadPreviewLabResult {
    bool saved_primary = false;
    bool loaded_ok = false;
    bool loaded_from_recovery = false;
    bool boot_safe_mode = false;
    SaveRecoveryTier recovery_tier = SaveRecoveryTier::None;
    std::string loaded_payload;
    std::string loaded_variables_payload;
    SaveSlotMeta loaded_meta;
    bool payload_matches_expected = false;
    std::vector<SaveLoadPreviewDiagnostic> diagnostics;
};

SaveLoadPreviewLabResult RunSaveLoadPreviewLab(const SaveLoadPreviewLabDocument& document,
                                               const std::filesystem::path& workspace_root);

} // namespace urpg::save
