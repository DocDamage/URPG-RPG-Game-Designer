#include "editor/achievement/achievement_panel.h"

namespace urpg::editor {

void AchievementPanel::bindRegistry(urpg::achievement::AchievementRegistry* registry) {
    m_registry = registry;
}

void AchievementPanel::render() {
    if (!m_registry) {
        m_snapshot = {
            {"panel", "achievement"},
            {"status", "disabled"},
            {"disabled_reason", "No AchievementRegistry is bound."},
            {"owner", "editor/achievement"},
            {"unlock_condition", "Bind AchievementRegistry before rendering the achievement panel."},
            {"total_count", 0},
            {"unlocked_count", 0},
            {"achievements", nlohmann::json::array()},
        };
        return;
    }

    const auto achievements = m_registry->getAllAchievements();
    nlohmann::json rows = nlohmann::json::array();
    size_t unlockedCount = 0;

    for (const auto& def : achievements) {
        auto progressOpt = m_registry->getProgress(def.id);
        bool unlocked = false;
        if (progressOpt.has_value()) {
            unlocked = progressOpt->unlocked;
        }
        if (unlocked) {
            ++unlockedCount;
        }

        nlohmann::json row{
            {"title", def.title},
            {"unlocked", unlocked}
        };
        rows.push_back(row);
    }

    m_snapshot = nlohmann::json{
        {"panel", "achievement"},
        {"status", "ready"},
        {"total_count", achievements.size()},
        {"unlocked_count", unlockedCount},
        {"trophy_export", m_registry->exportTrophyPayload()["summary"]},
        {"achievements", rows}
    };
}

nlohmann::json AchievementPanel::lastRenderSnapshot() const {
    return m_snapshot;
}

} // namespace urpg::editor
