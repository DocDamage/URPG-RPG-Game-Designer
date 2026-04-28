#include "editor/relationship/relationship_panel.h"

#include <utility>

namespace urpg::editor {

void RelationshipPanel::setRegistry(urpg::relationship::RelationshipRegistry registry) {
    registry_ = std::move(registry);
}

void RelationshipPanel::bindAffinityDocument(urpg::relationship::RelationshipAffinityDocument document) {
    document_ = std::move(document);
}

void RelationshipPanel::setPreviewEvent(urpg::relationship::AffinityEvent event) {
    preview_event_ = std::move(event);
}

bool RelationshipPanel::applyPreviewEvent() {
    if (!document_.has_value()) {
        return false;
    }
    const auto preview = document_->apply(registry_, preview_event_);
    return preview.diagnostics.empty() && !preview.applied_rule_ids.empty();
}

void RelationshipPanel::render() {
    snapshot_ = {
        {"panel", "relationship"},
        {"registry", registry_.serialize()},
        {"available_content", registry_.availableContent()},
    };
    if (document_.has_value()) {
        snapshot_["affinity_document"] = document_->toJson();
        snapshot_["preview_event"] = {{"source", preview_event_.source}, {"tag", preview_event_.tag}};
        snapshot_["preview"] = urpg::relationship::affinityPreviewToJson(document_->preview(registry_, preview_event_));
    }
}

nlohmann::json RelationshipPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
