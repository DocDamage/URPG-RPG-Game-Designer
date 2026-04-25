#include "editor/quest/quest_panel.h"

#include <utility>

namespace urpg::editor {

void QuestPanel::setRegistry(urpg::quest::QuestRegistry registry) {
    registry_ = std::move(registry);
}

void QuestPanel::render() {
    snapshot_ = {{"panel", "quest"}, {"registry", registry_.serialize()}};
}

nlohmann::json QuestPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
