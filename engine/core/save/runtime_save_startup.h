#pragma once

#include "engine/core/save/save_catalog.h"
#include "engine/core/save/save_recovery.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/save/save_types.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace urpg {

struct RuntimeSaveSlotPaths {
    int32_t slot_id = -1;
    std::filesystem::path primary_save_path;
    std::filesystem::path autosave_path;
    std::filesystem::path metadata_path;
    std::filesystem::path variables_path;
};

struct RuntimeSaveStartupEntry {
    int32_t slot_id = -1;
    SaveSlotMeta meta;
    RuntimeSaveSlotPaths paths;
    bool has_primary_payload = false;
    bool has_metadata_payload = false;
    bool has_variables_payload = false;
    bool loadable = false;
    bool requires_recovery = false;
    SaveRecoveryTier expected_recovery_tier = SaveRecoveryTier::None;
    std::string diagnostic;
};

struct RuntimeSaveStartupState {
    std::filesystem::path save_root;
    std::vector<RuntimeSaveStartupEntry> entries;

    bool hasLoadableSave() const;
    const RuntimeSaveStartupEntry* newestLoadableSave() const;
    std::string continueDisabledReason() const;
};

struct RuntimeSaveContinueResult {
    bool ok = false;
    int32_t slot_id = -1;
    bool boot_safe_mode = false;
    bool loaded_from_recovery = false;
    SaveRecoveryTier recovery_tier = SaveRecoveryTier::None;
    std::string error;
    SaveSlotMeta active_meta;
    std::vector<std::string> diagnostics;
};

std::filesystem::path defaultRuntimeSaveRoot(const std::filesystem::path& project_root);
RuntimeSaveStartupState discoverRuntimeSaves(const std::filesystem::path& project_root);
RuntimeSaveContinueResult continueNewestRuntimeSave(const RuntimeSaveStartupState& state);
RuntimeSaveLoadRequest makeRuntimeSaveLoadRequest(const RuntimeSaveSlotPaths& paths);

} // namespace urpg
