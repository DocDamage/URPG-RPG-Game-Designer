#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::achievement {

struct AchievementPlatformUpdate {
    std::string platform;
    std::string achievementId;
    uint32_t current = 0;
    uint32_t target = 0;
    bool unlocked = false;
    std::optional<std::string> unlockTime;
};

struct AchievementPlatformResult {
    bool success = false;
    std::string platform;
    std::string achievementId;
    std::string message;
};

class IAchievementPlatformBackend {
public:
    virtual ~IAchievementPlatformBackend() = default;
    virtual std::string platformId() const = 0;
    virtual AchievementPlatformResult submitProgress(const AchievementPlatformUpdate& update) = 0;
    virtual nlohmann::json snapshot() const = 0;
};

class MemoryAchievementPlatformBackend final : public IAchievementPlatformBackend {
public:
    explicit MemoryAchievementPlatformBackend(std::string platform = "urpg-local");

    std::string platformId() const override { return m_platform; }
    AchievementPlatformResult submitProgress(const AchievementPlatformUpdate& update) override;
    nlohmann::json snapshot() const override;

private:
    std::string m_platform;
    std::unordered_map<std::string, AchievementPlatformUpdate> m_updates;
};

class CommandAchievementPlatformBackend final : public IAchievementPlatformBackend {
public:
    CommandAchievementPlatformBackend(std::string platform, std::string executable, std::vector<std::string> arguments = {});

    std::string platformId() const override { return m_platform; }
    AchievementPlatformResult submitProgress(const AchievementPlatformUpdate& update) override;
    nlohmann::json snapshot() const override;

private:
    std::string m_platform;
    std::string m_executable;
    std::vector<std::string> m_arguments;
    std::vector<AchievementPlatformResult> m_results;
};

} // namespace urpg::achievement
