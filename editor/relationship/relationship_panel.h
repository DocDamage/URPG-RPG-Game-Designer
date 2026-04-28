#pragma once

#include "engine/core/relationship/relationship_affinity.h"
#include "engine/core/relationship/relationship_registry.h"

#include <nlohmann/json.hpp>
#include <optional>

namespace urpg::editor {

class RelationshipPanel {
public:
    void setRegistry(urpg::relationship::RelationshipRegistry registry);
    void bindAffinityDocument(urpg::relationship::RelationshipAffinityDocument document);
    void setPreviewEvent(urpg::relationship::AffinityEvent event);
    bool applyPreviewEvent();
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::relationship::RelationshipRegistry registry_;
    std::optional<urpg::relationship::RelationshipAffinityDocument> document_;
    urpg::relationship::AffinityEvent preview_event_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
