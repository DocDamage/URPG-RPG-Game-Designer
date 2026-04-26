#pragma once

#include "engine/core/character/character_save_state.h"
#include "engine/core/save/save_recovery.h"
#include "engine/core/save/save_types.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace urpg {

struct RuntimeSaveLoadRequest {
    std::filesystem::path primary_save_path;
    std::filesystem::path autosave_path;
    std::filesystem::path metadata_path;
    std::filesystem::path variables_path;
    bool force_safe_mode = false;
    std::string safe_mode_fallback_map = "Safe Mode - Origin";
};

struct RuntimeSaveLoadResult {
    bool ok = false;
    bool loaded_from_recovery = false;
    bool boot_safe_mode = false;
    SaveRecoveryTier recovery_tier = SaveRecoveryTier::None;
    std::string error;

    std::string payload;
    std::string variables_payload;
    SaveSlotMeta active_meta;
    std::optional<character::CreatedProtagonistSaveState> created_protagonist;
    std::vector<std::string> diagnostics;
};

class RuntimeSaveLoader {
  public:
    static RuntimeSaveLoadResult Load(const RuntimeSaveLoadRequest& request);
    static bool Save(const RuntimeSaveLoadRequest& request, const std::string& payload);
    static bool SaveCreatedProtagonist(const RuntimeSaveLoadRequest& request, const std::string& payload,
                                       EntityID entity, const character::CharacterIdentity& identity,
                                       std::vector<std::string>* diagnostics = nullptr);
};

} // namespace urpg
