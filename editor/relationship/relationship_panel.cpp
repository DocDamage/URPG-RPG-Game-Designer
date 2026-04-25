#include "editor/relationship/relationship_panel.h"

#include <utility>

namespace urpg::editor {

void RelationshipPanel::setRegistry(urpg::relationship::RelationshipRegistry registry) {
    registry_ = std::move(registry);
}

void RelationshipPanel::render() {
    snapshot_ = {
        {"panel", "relationship"},
        {"registry", registry_.serialize()},
        {"available_content", registry_.availableContent()},
    };
}

nlohmann::json RelationshipPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
