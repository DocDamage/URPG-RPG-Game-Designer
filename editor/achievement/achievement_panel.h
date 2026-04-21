#pragma once

#include "engine/core/achievement/achievement_registry.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

class AchievementPanel {
public:
    void bindRegistry(urpg::achievement::AchievementRegistry* registry);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::achievement::AchievementRegistry* m_registry = nullptr;
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
