#pragma once

#include "engine/core/achievement/achievement_registry.h"
#include "engine/core/release/provider_profile_status.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace urpg::achievement {

enum class AchievementPlatformProfileBackendType { Memory, Command, Unsupported };

struct AchievementPlatformProfileBackend {
    AchievementPlatformProfileBackend() = default;
    AchievementPlatformProfileBackend(std::string platformId_,
                                      AchievementPlatformProfileBackendType type_,
                                      std::string executable_,
                                      std::vector<std::string> arguments_)
        : platformId(std::move(platformId_)),
          type(type_),
          executable(std::move(executable_)),
          arguments(std::move(arguments_)) {}

    std::string platformId;
    AchievementPlatformProfileBackendType type = AchievementPlatformProfileBackendType::Memory;
    std::string executable;
    std::vector<std::string> arguments;
    std::string rawType;
};

struct AchievementPlatformProfile {
    std::string profileId;
    std::string packageId;
    std::vector<AchievementPlatformProfileBackend> backends;
    bool reviewed = false;
    std::string reviewedBy;
    std::string reviewedAt;
    std::string lastTestResult = "not_run";

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
urpg::release::ProviderProfileStatus achievementPlatformProfileStatus(
    const AchievementPlatformProfile& profile);
AchievementPlatformProfileApplyResult applyAchievementPlatformProfile(
    AchievementRegistry& registry, const AchievementPlatformProfile& profile);
nlohmann::json achievementPlatformProfileApplyResultToJson(const AchievementPlatformProfileApplyResult& result);

} // namespace urpg::achievement
