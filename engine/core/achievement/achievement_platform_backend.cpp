#include "engine/core/achievement/achievement_platform_backend.h"

#include "engine/core/platform/process_runner.h"

#include <chrono>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <utility>

namespace urpg::achievement {

namespace {

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

    std::vector<std::string> arguments;
    for (const auto& argument : m_arguments) {
        if (argument == "{payload}") {
            arguments.push_back(payloadPath.string());
        } else if (argument == "{achievementId}") {
            arguments.push_back(addressed.achievementId);
        } else if (argument == "{platform}") {
            arguments.push_back(m_platform);
        } else {
            arguments.push_back(argument);
        }
    }
    if (std::find(m_arguments.begin(), m_arguments.end(), "{payload}") == m_arguments.end()) {
        arguments.push_back(payloadPath.string());
    }

    urpg::platform::ProcessCommand command;
    command.executable = m_executable;
    command.arguments = std::move(arguments);
    command.captureStdout = false;
    command.captureStderr = false;
    const auto processResult = urpg::platform::runProcess(command);
    std::error_code ec;
    std::filesystem::remove(payloadPath, ec);

    AchievementPlatformResult result{
        processResult.exitCode == 0 && !processResult.timedOut && processResult.error.empty(), m_platform,
        update.achievementId,
        processResult.exitCode == 0 && !processResult.timedOut && processResult.error.empty()
            ? "Achievement progress submitted."
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
