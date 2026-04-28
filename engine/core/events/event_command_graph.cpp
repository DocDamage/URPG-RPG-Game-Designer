#include "engine/core/events/event_command_graph.h"

#include <algorithm>
#include <map>
#include <set>
#include <utility>

namespace urpg::events {
namespace {

std::string triggerToString(EventTrigger trigger) {
    switch (trigger) {
    case EventTrigger::PlayerTouch:
        return "player_touch";
    case EventTrigger::Autorun:
        return "autorun";
    case EventTrigger::Parallel:
        return "parallel";
    case EventTrigger::ActionButton:
        return "action_button";
    }
    return "action_button";
}

EventTrigger triggerFromString(const std::string& value) {
    if (value == "player_touch") {
        return EventTrigger::PlayerTouch;
    }
    if (value == "autorun") {
        return EventTrigger::Autorun;
    }
    if (value == "parallel") {
        return EventTrigger::Parallel;
    }
    return EventTrigger::ActionButton;
}

nlohmann::json nodeToJson(const EventCommandGraphNode& node) {
    return {
        {"id", node.id},
        {"label", node.label},
        {"kind", toString(node.kind)},
        {"target", node.target},
        {"value", node.value},
        {"amount", node.amount},
        {"x", node.x},
        {"y", node.y},
        {"payload", node.payload},
    };
}

EventCommandGraphNode nodeFromJson(const nlohmann::json& json) {
    EventCommandGraphNode node;
    if (!json.is_object()) {
        return node;
    }
    node.id = json.value("id", "");
    node.label = json.value("label", "");
    node.kind = eventCommandKindFromString(json.value("kind", "unsupported"));
    node.target = json.value("target", "");
    node.value = json.value("value", "");
    node.amount = json.value("amount", int64_t{0});
    node.x = json.value("x", 0);
    node.y = json.value("y", 0);
    node.payload = json.value("payload", nlohmann::json::object());
    return node;
}

EventCommand toEventCommand(const EventCommandGraphNode& node) {
    return EventCommand{node.id, node.kind, node.target, node.value, node.amount, {}, {}, node.payload};
}

const EventCommandGraphNode* findNode(const std::vector<EventCommandGraphNode>& nodes, const std::string& id) {
    const auto it = std::find_if(nodes.begin(), nodes.end(), [&id](const auto& node) {
        return node.id == id;
    });
    return it == nodes.end() ? nullptr : &(*it);
}

bool edgeConditionMatches(const EventCommandGraphEdge& edge, const EventWorldState& state) {
    if (!edge.condition_switch_id.empty()) {
        const auto it = state.switches.find(edge.condition_switch_id);
        const bool value = it != state.switches.end() && it->second;
        if (value != edge.condition_switch_value) {
            return false;
        }
    }
    if (!edge.condition_variable_id.empty()) {
        const auto it = state.variables.find(edge.condition_variable_id);
        const int64_t value = it == state.variables.end() ? 0 : it->second;
        if (value < edge.condition_variable_min) {
            return false;
        }
    }
    return true;
}

void applyCommandToState(const EventCommand& command, EventWorldState& state) {
    if (command.kind == EventCommandKind::Switch && !command.target.empty()) {
        state.switches[command.target] = command.value == "true" || command.value == "on" || command.amount != 0;
    } else if (command.kind == EventCommandKind::Variable && !command.target.empty()) {
        state.variables[command.target] = command.amount;
    }
}

std::string commandTrace(const EventCommand& command) {
    std::string trace = "execute:" + command.id + ":" + toString(command.kind);
    if (!command.target.empty()) {
        trace += ":" + command.target;
    }
    if (command.kind == EventCommandKind::Switch || command.kind == EventCommandKind::Variable) {
        trace += "=" + (command.kind == EventCommandKind::Switch ? command.value : std::to_string(command.amount));
    }
    return trace;
}

} // namespace

std::vector<EventCommandGraphDiagnostic> EventCommandGraphDocument::validate() const {
    std::vector<EventCommandGraphDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_graph_id", "Event command graph requires an id.", ""});
    }
    if (map_id.empty()) {
        diagnostics.push_back({"missing_map_id", "Event command graph requires a map id.", ""});
    }
    if (event_id.empty()) {
        diagnostics.push_back({"missing_event_id", "Event command graph requires an event id.", ""});
    }
    if (page_id.empty()) {
        diagnostics.push_back({"missing_page_id", "Event command graph requires a page id.", ""});
    }
    if (nodes.empty()) {
        diagnostics.push_back({"missing_nodes", "Event command graph requires at least one command node.", ""});
    }

    std::set<std::string> node_ids;
    for (const auto& node : nodes) {
        if (node.id.empty()) {
            diagnostics.push_back({"missing_node_id", "Event command graph node requires an id.", ""});
        } else if (!node_ids.insert(node.id).second) {
            diagnostics.push_back({"duplicate_node_id", "Event command graph node id must be unique.", node.id});
        }
        if (node.kind == EventCommandKind::Unsupported) {
            diagnostics.push_back({"unsupported_command_kind", "Event command graph node uses an unsupported command kind.",
                                   node.id});
        }
        if ((node.kind == EventCommandKind::Switch || node.kind == EventCommandKind::Variable ||
             node.kind == EventCommandKind::Transfer || node.kind == EventCommandKind::CommonEvent ||
             node.kind == EventCommandKind::Plugin) &&
            node.target.empty()) {
            diagnostics.push_back({"missing_command_target", "Event command graph node requires a command target.",
                                   node.id});
        }
    }

    if (!entry_node_id.empty() && !node_ids.contains(entry_node_id)) {
        diagnostics.push_back({"missing_entry_node", "Event command graph entry node does not exist.", entry_node_id});
    }

    std::set<std::string> edge_ids;
    std::map<std::string, size_t> incoming;
    for (const auto& edge : edges) {
        if (edge.id.empty()) {
            diagnostics.push_back({"missing_edge_id", "Event command graph edge requires an id.", ""});
        } else if (!edge_ids.insert(edge.id).second) {
            diagnostics.push_back({"duplicate_edge_id", "Event command graph edge id must be unique.", edge.id});
        }
        if (!node_ids.contains(edge.from_node_id)) {
            diagnostics.push_back({"missing_edge_source", "Event command graph edge source node does not exist.",
                                   edge.from_node_id});
        }
        if (!node_ids.contains(edge.to_node_id)) {
            diagnostics.push_back({"missing_edge_target", "Event command graph edge target node does not exist.",
                                   edge.to_node_id});
        }
        if (edge.kind != "sequence" && edge.kind != "conditional") {
            diagnostics.push_back({"unsupported_edge_kind", "Event command graph edge kind is unsupported.", edge.id});
        }
        if (edge.kind == "conditional" && edge.condition_switch_id.empty() && edge.condition_variable_id.empty()) {
            diagnostics.push_back({"missing_edge_condition", "Conditional event graph edge requires a condition.",
                                   edge.id});
        }
        ++incoming[edge.to_node_id];
    }
    if (entry_node_id.empty() && !nodes.empty()) {
        const auto roots = std::count_if(nodes.begin(), nodes.end(), [&incoming](const auto& node) {
            return incoming[node.id] == 0;
        });
        if (roots != 1) {
            diagnostics.push_back({"ambiguous_entry_node", "Event command graph requires one entry node.", ""});
        }
    }

    const auto ordered = orderedNodes();
    if (ordered.size() < nodes.size() && diagnostics.empty()) {
        diagnostics.push_back({"unreachable_or_cyclic_nodes", "Event command graph contains unreachable nodes or a cycle.",
                               id});
    }
    return diagnostics;
}

std::vector<EventCommandGraphNode> EventCommandGraphDocument::orderedNodes() const {
    std::vector<EventCommandGraphNode> ordered;
    if (nodes.empty()) {
        return ordered;
    }

    std::map<std::string, std::vector<std::string>> outgoing;
    std::map<std::string, size_t> incoming;
    for (const auto& node : nodes) {
        incoming[node.id] = 0;
    }
    for (const auto& edge : edges) {
        outgoing[edge.from_node_id].push_back(edge.to_node_id);
        ++incoming[edge.to_node_id];
    }
    for (auto& [_, targets] : outgoing) {
        std::sort(targets.begin(), targets.end());
    }

    std::string cursor = entry_node_id;
    if (cursor.empty()) {
        const auto root = std::find_if(nodes.begin(), nodes.end(), [&incoming](const auto& node) {
            return incoming[node.id] == 0;
        });
        if (root == nodes.end()) {
            return ordered;
        }
        cursor = root->id;
    }

    std::set<std::string> visited;
    while (!cursor.empty() && !visited.contains(cursor)) {
        const auto* node = findNode(nodes, cursor);
        if (node == nullptr) {
            break;
        }
        visited.insert(cursor);
        ordered.push_back(*node);
        const auto next_it = outgoing.find(cursor);
        if (next_it == outgoing.end() || next_it->second.empty()) {
            break;
        }
        cursor = next_it->second.front();
    }
    return ordered;
}

EventDocument EventCommandGraphDocument::toEventDocument() const {
    EventDocument document;
    document.addMap(MapDefinition{map_id, std::max(event_x + 1, 1), std::max(event_y + 1, 1)});

    EventPage page;
    page.id = page_id;
    page.trigger = trigger;
    page.priority = 0;
    for (const auto& node : orderedNodes()) {
        page.commands.push_back(toEventCommand(node));
    }

    EventDefinition event;
    event.id = event_id;
    event.map_id = map_id;
    event.x = event_x;
    event.y = event_y;
    event.pages.push_back(std::move(page));
    document.addEvent(std::move(event));
    return document;
}

nlohmann::json EventCommandGraphDocument::toJson() const {
    nlohmann::json json;
    json["schema"] = "urpg.event_command_graph.v1";
    json["id"] = id;
    json["map_id"] = map_id;
    json["event_id"] = event_id;
    json["page_id"] = page_id;
    json["trigger"] = triggerToString(trigger);
    json["event_x"] = event_x;
    json["event_y"] = event_y;
    json["entry_node_id"] = entry_node_id;
    json["nodes"] = nlohmann::json::array();
    for (const auto& node : nodes) {
        json["nodes"].push_back(nodeToJson(node));
    }
    json["edges"] = nlohmann::json::array();
    for (const auto& edge : edges) {
        json["edges"].push_back({
            {"id", edge.id},
            {"from_node_id", edge.from_node_id},
            {"to_node_id", edge.to_node_id},
            {"kind", edge.kind},
            {"condition_switch_id", edge.condition_switch_id},
            {"condition_switch_value", edge.condition_switch_value},
            {"condition_variable_id", edge.condition_variable_id},
            {"condition_variable_min", edge.condition_variable_min},
        });
    }
    return json;
}

EventCommandGraphDocument EventCommandGraphDocument::fromJson(const nlohmann::json& json) {
    EventCommandGraphDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    document.map_id = json.value("map_id", "");
    document.event_id = json.value("event_id", "");
    document.page_id = json.value("page_id", "page");
    document.trigger = triggerFromString(json.value("trigger", "action_button"));
    document.event_x = json.value("event_x", 0);
    document.event_y = json.value("event_y", 0);
    document.entry_node_id = json.value("entry_node_id", "");
    for (const auto& node_json : json.value("nodes", nlohmann::json::array())) {
        document.nodes.push_back(nodeFromJson(node_json));
    }
    for (const auto& edge_json : json.value("edges", nlohmann::json::array())) {
        if (!edge_json.is_object()) {
            continue;
        }
        document.edges.push_back({
            edge_json.value("id", ""),
            edge_json.value("from_node_id", ""),
            edge_json.value("to_node_id", ""),
            edge_json.value("kind", "sequence"),
            edge_json.value("condition_switch_id", ""),
            edge_json.value("condition_switch_value", true),
            edge_json.value("condition_variable_id", ""),
            edge_json.value("condition_variable_min", int64_t{0}),
        });
    }
    return document;
}

EventCommandGraphRuntimeResult ExecuteEventCommandGraphPreview(const EventCommandGraphDocument& document,
                                                               EventWorldState initial_state) {
    EventCommandGraphRuntimeResult result;
    result.state = std::move(initial_state);
    result.diagnostics = document.validate();
    if (!result.diagnostics.empty()) {
        return result;
    }

    urpg::EventInvocation invocation;
    invocation.event_id = document.event_id;
    invocation.priority = urpg::EventPriority::Normal;
    urpg::EventDispatchSession session;
    session.BeginInvocation(invocation);

    std::map<std::string, std::vector<EventCommandGraphEdge>> outgoing;
    for (const auto& edge : document.edges) {
        outgoing[edge.from_node_id].push_back(edge);
    }
    for (auto& [_, edges] : outgoing) {
        std::sort(edges.begin(), edges.end(), [](const auto& left, const auto& right) {
            if (left.kind != right.kind) {
                return left.kind == "conditional";
            }
            return left.id < right.id;
        });
    }

    auto ordered = document.orderedNodes();
    std::string cursor = ordered.empty() ? "" : ordered.front().id;
    std::set<std::string> visited;
    while (!cursor.empty() && !visited.contains(cursor)) {
        const auto* node = findNode(document.nodes, cursor);
        if (node == nullptr) {
            break;
        }
        visited.insert(cursor);
        auto command = toEventCommand(*node);
        result.executed_commands.push_back(command);
        result.runtime_trace.push_back(commandTrace(command));
        applyCommandToState(command, result.state);

        const auto next_edges = outgoing.find(cursor);
        if (next_edges == outgoing.end()) {
            break;
        }
        std::string next_node_id;
        for (const auto& edge : next_edges->second) {
            const bool condition_matches = edgeConditionMatches(edge, result.state);
            result.runtime_trace.push_back(std::string("edge:") + edge.id + ":" +
                                           (condition_matches ? "taken" : "skipped"));
            if (condition_matches) {
                result.traversed_edge_ids.push_back(edge.id);
                next_node_id = edge.to_node_id;
                break;
            }
        }
        cursor = next_node_id;
    }

    session.EndInvocation(invocation);
    result.timeline = session.Timeline();
    return result;
}

} // namespace urpg::events
