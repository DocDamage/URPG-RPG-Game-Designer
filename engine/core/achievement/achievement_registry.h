#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::achievement {

struct AchievementDef {
    std::string id;
    std::string title;
    std::string description;
    bool secret = false;
    std::string unlockCondition;
    std::string iconId;
    /** @brief Explicit target override. If zero, parsed from unlockCondition at registration time. */
    uint32_t target = 0;
};

struct AchievementProgress {
    uint32_t current = 0;
    uint32_t target = 0;
    bool unlocked = false;
    std::optional<std::string> unlockTime;
};

class AchievementRegistry {
public:
    void registerAchievement(const AchievementDef& def);
    std::optional<AchievementDef> getAchievement(const std::string& id) const;
    std::vector<AchievementDef> getAllAchievements() const;

    bool reportProgress(const std::string& id, uint32_t increment);
    std::optional<AchievementProgress> getProgress(const std::string& id) const;
    void resetProgress(const std::string& id);

    nlohmann::json saveToJson() const;
    void loadFromJson(const nlohmann::json& json);

private:
    std::unordered_map<std::string, AchievementDef> m_definitions;
    std::unordered_map<std::string, AchievementProgress> m_progress;
};

} // namespace urpg::achievement
