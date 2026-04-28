#pragma once

#include "engine/core/achievement/achievement_registry.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::achievement {

enum class AchievementPlatformProfileBackendType { Memory, Command };

struct AchievementPlatformProfileBackend {
    std::string platformId;
    AchievementPlatformProfileBackendType type = AchievementPlatformProfileBackendType::Memory;
    std::string executable;
    std::vector<std::string> arguments;
};

struct AchievementPlatformProfile {
    std::string profileId;
    std::string packageId;
    std::vector<AchievementPlatformProfileBackend> backends;

    nlohmann::json toJson() const;
    static AchievementPlatformProfile fromJson(const nlohmann::json& json);
};

struct AchievementPlatformProfileDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AchievementPlatformProfileApplyResult {
    bool applied = false;
    std::size_t backendCount = 0;
    std::vector<AchievementPlatformProfileDiagnostic> diagnostics;
};

const char* achievementPlatformBackendTypeName(AchievementPlatformProfileBackendType type);
AchievementPlatformProfileBackendType achievementPlatformBackendTypeFromString(const std::string& value);
std::vector<AchievementPlatformProfileDiagnostic> validateAchievementPlatformProfile(
    const AchievementPlatformProfile& profile);
AchievementPlatformProfileApplyResult applyAchievementPlatformProfile(
    AchievementRegistry& registry, const AchievementPlatformProfile& profile);
nlohmann::json achievementPlatformProfileApplyResultToJson(const AchievementPlatformProfileApplyResult& result);

} // namespace urpg::achievement
