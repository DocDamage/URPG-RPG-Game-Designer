#pragma once

#include "engine/core/save/save_recovery.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::save {

struct SaveCorruptionResult {
    bool success = false;
    std::filesystem::path corrupted_copy_path;
    std::vector<std::string> diagnostics;
};

class SaveCorruptionLab {
public:
    SaveCorruptionResult corruptFixtureCopy(const std::filesystem::path& source_fixture,
                                            const std::filesystem::path& lab_directory,
                                            const std::string& mode) const;
    nlohmann::json simulateRecovery(const urpg::SaveRecoveryRequest& request) const;
};

} // namespace urpg::save
