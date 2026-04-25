#pragma once

#include "engine/core/quest/quest_registry.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class QuestPanel {
public:
    void setRegistry(urpg::quest::QuestRegistry registry);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::quest::QuestRegistry registry_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
