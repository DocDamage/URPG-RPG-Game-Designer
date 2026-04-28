#pragma once

#include "engine/core/events/event_document.h"
#include "engine/core/events/event_runtime.h"

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::events {

struct EventCommandGraphNode {
    std::string id;
    std::string label;
    EventCommandKind kind = EventCommandKind::Unsupported;
    std::string target;
    std::string value;
    int64_t amount = 0;
    int32_t x = 0;
    int32_t y = 0;
    nlohmann::json payload = nlohmann::json::object();
};

struct EventCommandGraphEdge {
    std::string id;
    std::string from_node_id;
    std::string to_node_id;
    std::string kind = "sequence";
    std::string condition_switch_id;
    bool condition_switch_value = true;
    std::string condition_variable_id;
    int64_t condition_variable_min = 0;
};

struct EventCommandGraphDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct EventCommandGraphDocument {
    std::string id;
    std::string map_id;
    std::string event_id;
    std::string page_id = "page";
    EventTrigger trigger = EventTrigger::ActionButton;
    int32_t event_x = 0;
    int32_t event_y = 0;
    std::string entry_node_id;
    std::vector<EventCommandGraphNode> nodes;
    std::vector<EventCommandGraphEdge> edges;

    std::vector<EventCommandGraphDiagnostic> validate() const;
    std::vector<EventCommandGraphNode> orderedNodes() const;
    EventDocument toEventDocument() const;
    nlohmann::json toJson() const;

    static EventCommandGraphDocument fromJson(const nlohmann::json& json);
};

struct EventCommandGraphRuntimeResult {
    EventWorldState state;
    std::vector<EventCommand> executed_commands;
    std::vector<std::string> traversed_edge_ids;
    std::vector<std::string> runtime_trace;
    urpg::EventExecutionTimeline timeline;
    std::vector<EventCommandGraphDiagnostic> diagnostics;
};

EventCommandGraphRuntimeResult ExecuteEventCommandGraphPreview(const EventCommandGraphDocument& document,
                                                               EventWorldState initial_state = {});

} // namespace urpg::events
