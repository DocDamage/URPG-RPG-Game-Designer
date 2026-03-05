#pragma once

#include "engine/core/save/save_types.h"

#include <filesystem>
#include <string>

namespace urpg {

enum class SaveRecoveryTier {
    None = 0,
    Level1Autosave = 1,
    Level2MetadataVariables = 2,
    Level3SafeSkeleton = 3
};

struct SaveRecoveryRequest {
    std::filesystem::path autosave_path;
    std::filesystem::path metadata_path;
    std::filesystem::path variables_path;
    std::string safe_mode_fallback_map = "Safe Mode - Origin";
};

struct SaveRecoveryResult {
    bool ok = false;
    SaveRecoveryTier tier = SaveRecoveryTier::None;
    std::string error;

    std::string full_payload;
    std::string metadata_payload;
    std::string variables_payload;

    SaveSlotMeta skeleton_meta;
    bool variables_reset = false;
};

class SaveRecoveryManager {
public:
    static SaveRecoveryResult Recover(const SaveRecoveryRequest& request);
};

} // namespace urpg
