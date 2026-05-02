#include "engine/core/achievement/achievement_platform_profile.h"

#include <memory>
#include <utility>

namespace urpg::achievement {
namespace {

std::vector<std::string> stringsFromJsonArray(const nlohmann::json& json) {
    std::vector<std::string> values;
    if (!json.is_array()) {
        return values;
    }
    for (const auto& value : json) {
        if (value.is_string()) {
            values.push_back(value.get<std::string>());
        }
    }
    return values;
}

} // namespace

const char* achievementPlatformBackendTypeName(AchievementPlatformProfileBackendType type) {
    switch (type) {
    case AchievementPlatformProfileBackendType::Memory:
        return "memory";
    case AchievementPlatformProfileBackendType::Command:
        return "command";
    case AchievementPlatformProfileBackendType::Unsupported:
        return "unsupported";
    }
    return "memory";
}

AchievementPlatformProfileBackendType achievementPlatformBackendTypeFromString(const std::string& value) {
    if (value == "command") {
        return AchievementPlatformProfileBackendType::Command;
    }
    if (value == "memory") {
        return AchievementPlatformProfileBackendType::Memory;
    }
    return AchievementPlatformProfileBackendType::Unsupported;
}

nlohmann::json AchievementPlatformProfile::toJson() const {
    nlohmann::json backendJson = nlohmann::json::array();
    for (const auto& backend : backends) {
        backendJson.push_back({
            {"platformId", backend.platformId},
            {"type", backend.type == AchievementPlatformProfileBackendType::Unsupported && !backend.rawType.empty()
                         ? backend.rawType
                         : achievementPlatformBackendTypeName(backend.type)},
            {"executable", backend.executable},
            {"arguments", backend.arguments},
        });
    }

    return {
        {"schema", "urpg.achievement_platform_profile.v1"},
        {"profileId", profileId},
        {"packageId", packageId},
        {"backends", backendJson},
        {"reviewed", reviewed},
        {"reviewedBy", reviewedBy},
        {"reviewedAt", reviewedAt},
        {"lastTestResult", lastTestResult},
    };
}

AchievementPlatformProfile AchievementPlatformProfile::fromJson(const nlohmann::json& json) {
    AchievementPlatformProfile profile;
    if (!json.is_object()) {
        return profile;
    }

    profile.profileId = json.value("profileId", "");
    profile.packageId = json.value("packageId", "");
    if (const auto backends = json.find("backends"); backends != json.end() && backends->is_array()) {
        for (const auto& item : *backends) {
            if (!item.is_object()) {
                continue;
            }
            AchievementPlatformProfileBackend backend;
            backend.platformId = item.value("platformId", "");
            backend.rawType = item.value("type", std::string("memory"));
            backend.type = achievementPlatformBackendTypeFromString(backend.rawType);
            backend.executable = item.value("executable", "");
            backend.arguments = stringsFromJsonArray(item.value("arguments", nlohmann::json::array()));
            profile.backends.push_back(std::move(backend));
        }
    }
    profile.reviewed = json.value("reviewed", false);
    profile.reviewedBy = json.value("reviewedBy", "");
    profile.reviewedAt = json.value("reviewedAt", "");
    profile.lastTestResult = json.value("lastTestResult", std::string("not_run"));
    return profile;
}

std::vector<AchievementPlatformProfileDiagnostic> validateAchievementPlatformProfile(
    const AchievementPlatformProfile& profile) {
    std::vector<AchievementPlatformProfileDiagnostic> diagnostics;
    if (profile.profileId.empty()) {
        diagnostics.push_back({"missing_profile_id", "Achievement platform profile requires a profile id.", ""});
    }
    if (profile.packageId.empty()) {
        diagnostics.push_back({"missing_package_id", "Achievement platform profile requires a package id.",
                               profile.profileId});
    }
    if (profile.backends.empty()) {
        diagnostics.push_back({"missing_backends", "Achievement platform profile requires at least one backend.",
                               profile.profileId});
    }
    for (const auto& backend : profile.backends) {
        if (backend.platformId.empty()) {
            diagnostics.push_back({"missing_platform_id", "Achievement platform backend requires a platform id.",
                                   profile.profileId});
        }
        if (backend.type == AchievementPlatformProfileBackendType::Unsupported) {
            diagnostics.push_back({"unsupported_provider", "Achievement platform backend type is not supported.",
                                   backend.platformId});
        }
        if (backend.type == AchievementPlatformProfileBackendType::Command && backend.executable.empty()) {
            diagnostics.push_back({"missing_command_executable",
                                   "Command achievement platform backend requires an executable.",
                                   backend.platformId});
        }
    }
    return diagnostics;
}

urpg::release::ProviderProfileStatus achievementPlatformProfileStatus(
    const AchievementPlatformProfile& profile) {
    urpg::release::ProviderProfileStatus status;
    status.lastTestResult = profile.lastTestResult.empty() ? "not_run" : profile.lastTestResult;

    if (profile.backends.empty()) {
        status.status = "disabled";
        return status;
    }

    bool hasUnsupported = false;
    bool hasCommand = false;
    bool missingCredentials = false;
    for (const auto& backend : profile.backends) {
        if (backend.type == AchievementPlatformProfileBackendType::Unsupported) {
            hasUnsupported = true;
        }
        if (backend.type == AchievementPlatformProfileBackendType::Command) {
            hasCommand = true;
            if (backend.executable.empty()) {
                missingCredentials = true;
            }
        }
    }

    if (hasUnsupported) {
        status.status = "unsupported_provider";
        return status;
    }
    if (missingCredentials) {
        status.status = "missing_credentials";
        status.credentialSourceCategory = "command";
        return status;
    }
    if (!hasCommand) {
        status.status = "dry_run";
        return status;
    }

    status.credentialSourceCategory = "command";
    status.reviewStatus = profile.reviewed ? "reviewed" : "unreviewed";
    status.status = profile.reviewed ? "configured_reviewed" : "configured_unreviewed";
    status.releasePackagingAllowed = profile.reviewed && status.lastTestResult == "pass";
    return status;
}

AchievementPlatformProfileApplyResult applyAchievementPlatformProfile(
    AchievementRegistry& registry, const AchievementPlatformProfile& profile) {
    AchievementPlatformProfileApplyResult result;
    result.diagnostics = validateAchievementPlatformProfile(profile);
    if (!result.diagnostics.empty()) {
        return result;
    }

    for (const auto& backend : profile.backends) {
        if (backend.type == AchievementPlatformProfileBackendType::Command) {
            registry.addPlatformBackend(std::make_shared<CommandAchievementPlatformBackend>(
                backend.platformId, backend.executable, backend.arguments));
        } else {
            registry.addPlatformBackend(std::make_shared<MemoryAchievementPlatformBackend>(backend.platformId));
        }
        ++result.backendCount;
    }

    result.applied = true;
    return result;
}

nlohmann::json achievementPlatformProfileApplyResultToJson(const AchievementPlatformProfileApplyResult& result) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : result.diagnostics) {
        diagnostics.push_back({
            {"code", diagnostic.code},
            {"message", diagnostic.message},
            {"target", diagnostic.target},
        });
    }
    return {
        {"applied", result.applied},
        {"backendCount", result.backendCount},
        {"diagnostics", diagnostics},
    };
}

} // namespace urpg::achievement
