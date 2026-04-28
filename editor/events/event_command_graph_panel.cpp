#include "editor/events/event_command_graph_panel.h"

#include "engine/core/events/event_dependency_graph.h"

#include <utility>

namespace urpg::editor {

void EventCommandGraphPanel::loadDocument(urpg::events::EventCommandGraphDocument document,
                                          urpg::events::EventWorldState initial_state) {
    document_ = std::move(document);
    initial_state_ = std::move(initial_state);
    loaded_ = true;
    refreshPreview();
}

void EventCommandGraphPanel::selectNode(std::string node_id) {
    snapshot_.selected_node_id = std::move(node_id);
}

void EventCommandGraphPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load an event command graph before rendering this panel.";
        return;
    }
    refreshPreview();
}

void EventCommandGraphPanel::refreshPreview() {
    runtime_document_ = document_.toEventDocument();
    runtime_preview_ = urpg::events::ExecuteEventCommandGraphPreview(document_, initial_state_);
    const auto dependencies = urpg::events::EventDependencyGraph::build(runtime_document_);

    snapshot_.disabled = false;
    snapshot_.graph_id = document_.id;
    snapshot_.node_count = document_.nodes.size();
    snapshot_.edge_count = document_.edges.size();
    snapshot_.runtime_command_count = runtime_preview_.executed_commands.size();
    snapshot_.diagnostic_count = runtime_preview_.diagnostics.size();
    snapshot_.dependency_edge_count = dependencies.edges().size();
    snapshot_.runtime_executed = snapshot_.diagnostic_count == 0 && !runtime_preview_.executed_commands.empty();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Event command graph preview is ready." : "Event command graph preview has diagnostics.";
}

} // namespace urpg::editor
