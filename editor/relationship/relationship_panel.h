#pragma once

#include "engine/core/relationship/relationship_registry.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class RelationshipPanel {
public:
    void setRegistry(urpg::relationship::RelationshipRegistry registry);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::relationship::RelationshipRegistry registry_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
