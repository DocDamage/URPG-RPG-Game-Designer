#include "editor/events/event_authoring_model.h"

namespace urpg::editor {

void EventAuthoringModel::load(events::EventDocument document, events::EventWorldState state) {
    document_ = std::move(document);
    state_ = std::move(state);
    refresh();
}

void EventAuthoringModel::clear() {
    document_ = {};
    state_ = {};
    graph_ = {};
    diagnostics_.clear();
    snapshot_ = {};
}

void EventAuthoringModel::startDebugging(const std::string& event_id) {
    debugger_.start(document_, event_id, state_);
    refresh();
}

bool EventAuthoringModel::stepDebugger() {
    const bool running = debugger_.step();
    refresh();
    return running;
}

void EventAuthoringModel::refresh() {
    graph_ = events::EventDependencyGraph::build(document_);
    diagnostics_ = document_.validate(state_);
    snapshot_ = {};
    snapshot_.event_count = document_.events().size();
    for (const auto& event : document_.events()) {
        if (event.drag.enabled) {
            ++snapshot_.draggable_event_count;
        }
        snapshot_.page_count += event.pages.size();
        snapshot_.has_active_page = snapshot_.has_active_page || document_.resolveActivePage(event.id, state_).has_value();
        for (const auto& page : event.pages) {
            snapshot_.command_count += page.commands.size();
        }
    }
    snapshot_.diagnostic_count = diagnostics_.size();
    snapshot_.dependency_edge_count = graph_.edges().size();
    snapshot_.debugger_running = debugger_.snapshot().running;
}

} // namespace urpg::editor
