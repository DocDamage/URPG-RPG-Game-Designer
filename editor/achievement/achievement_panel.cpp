#include "editor/achievement/achievement_panel.h"

namespace urpg::editor {

void AchievementPanel::bindRegistry(urpg::achievement::AchievementRegistry* registry) {
    m_registry = registry;
}

void AchievementPanel::render() {
    if (!m_registry) {
        m_snapshot = nlohmann::json::object();
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
        {"total_count", achievements.size()},
        {"unlocked_count", unlockedCount},
        {"achievements", rows}
    };
}

nlohmann::json AchievementPanel::lastRenderSnapshot() const {
    return m_snapshot;
}

} // namespace urpg::editor
