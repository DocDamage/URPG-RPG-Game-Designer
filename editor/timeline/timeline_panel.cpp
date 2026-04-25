#include "editor/timeline/timeline_panel.h"

#include <utility>

namespace urpg::editor {

void TimelinePanel::setDocument(timeline::TimelineDocument document) {
    document_ = std::move(document);
    has_document_ = true;
}

void TimelinePanel::clearDocument() {
    document_ = timeline::TimelineDocument{};
    has_document_ = false;
}

TimelinePanelSnapshot TimelinePanel::snapshot() const {
    TimelinePanelSnapshot snapshot;
    snapshot.has_document = has_document_;
    if (!has_document_) {
        return snapshot;
    }

    snapshot.actor_count = document_.actors().size();
    snapshot.command_count = document_.commands().size();
    for (const auto& diagnostic : document_.validate()) {
        snapshot.diagnostics.push_back(diagnostic.code + ":" + diagnostic.command_id);
    }
    return snapshot;
}

void TimelinePanel::render() {
    last_render_snapshot_ = snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
