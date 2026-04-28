#include "engine/core/achievement/achievement_platform_backend.h"

#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace urpg::achievement {

namespace {

std::string quoteCommandArg(const std::string& value) {
#ifdef _WIN32
    std::string quoted = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted += ch;
        }
    }
    quoted += "\"";
    return quoted;
#else
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
#endif
}

std::filesystem::path writeUpdatePayload(const AchievementPlatformUpdate& update) {
    const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("urpg_achievement_platform_" + unique + ".json");
    nlohmann::json payload{
        {"platform", update.platform},
        {"achievementId", update.achievementId},
        {"current", update.current},
        {"target", update.target},
        {"unlocked", update.unlocked},
    };
    if (update.unlockTime.has_value()) {
        payload["unlockTime"] = *update.unlockTime;
    }

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return {};
    }
    out << payload.dump();
    return out.good() ? path : std::filesystem::path{};
}

} // namespace

MemoryAchievementPlatformBackend::MemoryAchievementPlatformBackend(std::string platform)
    : m_platform(std::move(platform)) {}

AchievementPlatformResult MemoryAchievementPlatformBackend::submitProgress(const AchievementPlatformUpdate& update) {
    if (update.achievementId.empty()) {
        return {false, m_platform, update.achievementId, "Achievement id is required for platform submission."};
    }
    if (!update.platform.empty() && update.platform != m_platform) {
        return {false, m_platform, update.achievementId, "Achievement update was addressed to a different platform."};
    }

    AchievementPlatformUpdate stored = update;
    stored.platform = m_platform;
    m_updates[stored.achievementId] = stored;
    return {true, m_platform, stored.achievementId, "Achievement progress submitted."};
}

nlohmann::json MemoryAchievementPlatformBackend::snapshot() const {
    nlohmann::json updates = nlohmann::json::array();
    for (const auto& [id, update] : m_updates) {
        nlohmann::json row{
            {"platform", m_platform},
            {"achievementId", id},
            {"current", update.current},
            {"target", update.target},
            {"unlocked", update.unlocked},
        };
        if (update.unlockTime.has_value()) {
            row["unlockTime"] = *update.unlockTime;
        }
        updates.push_back(std::move(row));
    }
    return {
        {"platform", m_platform},
        {"submittedCount", updates.size()},
        {"updates", updates},
    };
}

CommandAchievementPlatformBackend::CommandAchievementPlatformBackend(std::string platform,
                                                                     std::string executable,
                                                                     std::vector<std::string> arguments)
    : m_platform(std::move(platform)), m_executable(std::move(executable)), m_arguments(std::move(arguments)) {}

AchievementPlatformResult CommandAchievementPlatformBackend::submitProgress(const AchievementPlatformUpdate& update) {
    if (m_executable.empty()) {
        AchievementPlatformResult result{false, m_platform, update.achievementId,
                                         "Platform command backend has no executable configured."};
        m_results.push_back(result);
        return result;
    }

    auto addressed = update;
    addressed.platform = m_platform;
    const auto payloadPath = writeUpdatePayload(addressed);
    if (payloadPath.empty()) {
        AchievementPlatformResult result{false, m_platform, update.achievementId,
                                         "Failed to write platform achievement payload."};
        m_results.push_back(result);
        return result;
    }

    std::ostringstream command;
    command << quoteCommandArg(m_executable);
    for (const auto& argument : m_arguments) {
        if (argument == "{payload}") {
            command << " " << quoteCommandArg(payloadPath.string());
        } else if (argument == "{achievementId}") {
            command << " " << quoteCommandArg(addressed.achievementId);
        } else if (argument == "{platform}") {
            command << " " << quoteCommandArg(m_platform);
        } else {
            command << " " << quoteCommandArg(argument);
        }
    }
    if (std::find(m_arguments.begin(), m_arguments.end(), "{payload}") == m_arguments.end()) {
        command << " " << quoteCommandArg(payloadPath.string());
    }
#ifdef _WIN32
    command << " >NUL 2>NUL";
#else
    command << " >/dev/null 2>/dev/null";
#endif

    const int exitCode = std::system(command.str().c_str());
    std::error_code ec;
    std::filesystem::remove(payloadPath, ec);

    AchievementPlatformResult result{exitCode == 0, m_platform, update.achievementId,
                                     exitCode == 0 ? "Achievement progress submitted."
                                                   : "Platform command backend returned failure."};
    m_results.push_back(result);
    return result;
}

nlohmann::json CommandAchievementPlatformBackend::snapshot() const {
    nlohmann::json results = nlohmann::json::array();
    for (const auto& result : m_results) {
        results.push_back({
            {"platform", result.platform},
            {"achievementId", result.achievementId},
            {"success", result.success},
            {"message", result.message},
        });
    }
    return {
        {"platform", m_platform},
        {"type", "command"},
        {"executable", m_executable},
        {"submittedCount", results.size()},
        {"results", results},
    };
}

} // namespace urpg::achievement
