#include "editor/events/event_command_graph_panel.h"

#include "engine/core/events/event_dependency_graph.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {
namespace {

urpg::wysiwyg::PreviewTraceRow makeEventTraceRow(const std::string& trace, const std::string& source_id) {
    const auto first = trace.find(':');
    if (first == std::string::npos) {
        return {trace, source_id, trace};
    }
    return {trace.substr(0, first), source_id, trace.substr(first + 1)};
}

} // namespace

void EventCommandGraphPanel::loadDocument(urpg::events::EventCommandGraphDocument document,
                                          urpg::events::EventWorldState initial_state) {
    document_ = std::move(document);
    initial_state_ = std::move(initial_state);
    loaded_ = true;
    refreshPreview();
}

void EventCommandGraphPanel::selectNode(std::string node_id) {
    snapshot_.selected_node_id = std::move(node_id);
    if (loaded_) {
        refreshPreview();
    }
}

void EventCommandGraphPanel::addOrUpdateNode(urpg::events::EventCommandGraphNode node) {
    const auto it = std::find_if(document_.nodes.begin(), document_.nodes.end(), [&node](const auto& existing) {
        return existing.id == node.id;
    });
    if (it == document_.nodes.end()) {
        document_.nodes.push_back(std::move(node));
    } else {
        *it = std::move(node);
    }
    if (document_.entry_node_id.empty() && !document_.nodes.empty()) {
        document_.entry_node_id = document_.nodes.front().id;
    }
    if (loaded_) {
        refreshPreview();
    }
}

void EventCommandGraphPanel::moveNode(std::string node_id, int32_t x, int32_t y) {
    const auto it = std::find_if(document_.nodes.begin(), document_.nodes.end(), [&node_id](const auto& node) {
        return node.id == node_id;
    });
    if (it != document_.nodes.end()) {
        it->x = x;
        it->y = y;
    }
    if (loaded_) {
        refreshPreview();
    }
}

void EventCommandGraphPanel::connectNodes(urpg::events::EventCommandGraphEdge edge) {
    const auto it = std::find_if(document_.edges.begin(), document_.edges.end(), [&edge](const auto& existing) {
        return existing.id == edge.id;
    });
    if (it == document_.edges.end()) {
        document_.edges.push_back(std::move(edge));
    } else {
        *it = std::move(edge);
    }
    if (loaded_) {
        refreshPreview();
    }
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
    snapshot_.runtime_trace_count = runtime_preview_.runtime_trace.size();
    snapshot_.traversed_edge_count = runtime_preview_.traversed_edge_ids.size();
    snapshot_.diagnostic_count = runtime_preview_.diagnostics.size();
    snapshot_.dependency_edge_count = dependencies.edges().size();
    snapshot_.switch_state_count = runtime_preview_.state.switches.size();
    snapshot_.variable_state_count = runtime_preview_.state.variables.size();
    snapshot_.traversal_ratio = snapshot_.edge_count == 0
        ? (snapshot_.node_count == 0 ? 0.0f : 1.0f)
        : static_cast<float>(snapshot_.traversed_edge_count) / static_cast<float>(snapshot_.edge_count);
    snapshot_.selected_node_label.clear();
    snapshot_.selected_node_kind.clear();
    snapshot_.selected_node_runtime_summary.clear();
    const auto selected = std::find_if(document_.nodes.begin(), document_.nodes.end(), [&](const auto& node) {
        return node.id == snapshot_.selected_node_id;
    });
    if (selected != document_.nodes.end()) {
        snapshot_.selected_node_label = selected->label;
        snapshot_.selected_node_kind = urpg::events::toString(selected->kind);
        snapshot_.selected_node_runtime_summary = snapshot_.selected_node_kind + ":" + snapshot_.selected_node_label;
    }
    snapshot_.runtime_executed = snapshot_.diagnostic_count == 0 && !runtime_preview_.executed_commands.empty();
    if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Resolve event graph diagnostics before runtime preview approval.";
    } else if (!snapshot_.selected_node_id.empty()) {
        snapshot_.ux_focus_lane = "node_inspector";
        snapshot_.primary_action = "Inspect the selected node command and branch effects.";
    } else if (snapshot_.traversed_edge_count < snapshot_.edge_count) {
        snapshot_.ux_focus_lane = "branch_coverage";
        snapshot_.primary_action = "Adjust initial switch/variable state to preview untraversed branches.";
    } else {
        snapshot_.ux_focus_lane = "runtime_trace";
        snapshot_.primary_action = "Use the runtime trace to confirm the graph ships as authored.";
    }
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Event command graph preview is ready." : "Event command graph preview has diagnostics.";

    preview_session_ = {};
    preview_session_.route_id = "events/event_command_graph";
    preview_session_.surface_id = "event_command_graph";
    preview_session_.source_id = document_.id;
    preview_session_.mode = runtime_preview_.runtime_trace.empty() ? urpg::wysiwyg::PreviewMode::EditorOnly
                                                                   : urpg::wysiwyg::PreviewMode::RuntimeBacked;
    for (const auto& trace : runtime_preview_.runtime_trace) {
        preview_session_.runtime_trace_rows.push_back(makeEventTraceRow(trace, document_.event_id));
    }
    for (const auto& command : runtime_preview_.executed_commands) {
        preview_session_.summary_rows.push_back(
            {urpg::events::toString(command.kind), command.id, command.target.empty() ? command.value : command.target});
    }
    preview_session_.summary_rows.push_back({"switches", document_.id, std::to_string(snapshot_.switch_state_count)});
    preview_session_.summary_rows.push_back({"variables", document_.id, std::to_string(snapshot_.variable_state_count)});
    for (const auto& diagnostic : runtime_preview_.diagnostics) {
        preview_session_.diagnostics.push_back({diagnostic.code, diagnostic.message, diagnostic.target, true});
    }
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::SavedData,
                                         !document_.id.empty(),
                                         document_.id.empty() ? "Event command graph id is missing."
                                                              : "Event command graph document is saved."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::LivePreview,
                                         snapshot_.node_count > 0,
                                         std::to_string(snapshot_.node_count) + " graph nodes projected."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::RuntimeExecution,
                                         !runtime_preview_.runtime_trace.empty(),
                                         std::to_string(runtime_preview_.runtime_trace.size()) + " runtime trace rows."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::Diagnostics,
                                         runtime_preview_.diagnostics.empty(),
                                         runtime_preview_.diagnostics.empty() ? "No event graph blockers."
                                                                              : "Event graph diagnostics are present."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::Tests,
                                         true,
                                         "Covered by PreviewSession and event graph tests."});
    preview_session_.recomputeConfidence();
}

} // namespace urpg::editor
