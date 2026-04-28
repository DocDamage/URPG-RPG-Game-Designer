#pragma once

#include "engine/core/achievement/achievement_registry.h"
#include "engine/core/achievement/achievement_platform_profile.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

class AchievementPanel {
public:
    void bindRegistry(urpg::achievement::AchievementRegistry* registry);
    void bindPlatformProfile(const urpg::achievement::AchievementPlatformProfile* profile);
    bool applyPlatformProfile();
    std::vector<urpg::achievement::AchievementPlatformResult> syncUnlockedAchievements();
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::achievement::AchievementRegistry* m_registry = nullptr;
    const urpg::achievement::AchievementPlatformProfile* m_platformProfile = nullptr;
    nlohmann::json m_lastAction = nlohmann::json::object();
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
