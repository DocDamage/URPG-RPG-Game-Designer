#include "engine/core/events/event_dependency_graph.h"

#include <algorithm>

namespace urpg::events {

namespace {

void addConditionEdges(std::vector<EventDependencyEdge>& edges, const std::string& source_id, const EventCondition& condition) {
    for (const auto& [id, _] : condition.switches) {
        edges.push_back({source_id, "switch", id, EventDependencyAccess::Read});
    }
    for (const auto& [id, _] : condition.min_variables) {
        edges.push_back({source_id, "variable", id, EventDependencyAccess::Read});
    }
    for (const auto& id : condition.quest_flags) {
        edges.push_back({source_id, "quest_flag", id, EventDependencyAccess::Read});
    }
}

void addCommandEdge(std::vector<EventDependencyEdge>& edges, const std::string& source_id, const EventCommand& command) {
    for (const auto& save_field : command.read_save_fields) {
        edges.push_back({source_id, "save_field", save_field, EventDependencyAccess::Read});
    }
    for (const auto& save_field : command.write_save_fields) {
        edges.push_back({source_id, "save_field", save_field, EventDependencyAccess::Write});
    }

    switch (command.kind) {
        case EventCommandKind::Switch:
            edges.push_back({source_id, "switch", command.target, EventDependencyAccess::Write});
            break;
        case EventCommandKind::Variable:
            edges.push_back({source_id, "variable", command.target, EventDependencyAccess::Write});
            break;
        case EventCommandKind::Transfer:
            edges.push_back({source_id, "map", command.target, EventDependencyAccess::Read});
            break;
        case EventCommandKind::CommonEvent:
            edges.push_back({source_id, "common_event", command.target, EventDependencyAccess::Call});
            break;
        case EventCommandKind::Plugin:
            edges.push_back({source_id, "plugin", command.target, EventDependencyAccess::Call});
            break;
        default:
            break;
    }
}

} // namespace

EventDependencyGraph EventDependencyGraph::build(const EventDocument& document) {
    EventDependencyGraph graph;
    for (const auto& event : document.events()) {
        for (const auto& page : event.pages) {
            const std::string source_id = event.id + "/" + page.id;
            addConditionEdges(graph.edges_, source_id, page.conditions);
            for (const auto& command : page.commands) {
                addCommandEdge(graph.edges_, source_id, command);
            }
        }
    }
    for (const auto& [id, common_event] : document.commonEvents()) {
        const std::string source_id = "common/" + id;
        for (const auto& command : common_event.commands) {
            addCommandEdge(graph.edges_, source_id, command);
        }
    }
    std::sort(graph.edges_.begin(), graph.edges_.end(), [](const EventDependencyEdge& left, const EventDependencyEdge& right) {
        if (left.source_id != right.source_id) {
            return left.source_id < right.source_id;
        }
        if (left.target_type != right.target_type) {
            return left.target_type < right.target_type;
        }
        if (left.target_id != right.target_id) {
            return left.target_id < right.target_id;
        }
        return static_cast<uint8_t>(left.access) < static_cast<uint8_t>(right.access);
    });
    return graph;
}

std::vector<EventDependencyEdge> EventDependencyGraph::edgesForSource(const std::string& source_id) const {
    std::vector<EventDependencyEdge> result;
    std::copy_if(edges_.begin(), edges_.end(), std::back_inserter(result), [&](const EventDependencyEdge& edge) {
        return edge.source_id == source_id;
    });
    return result;
}

} // namespace urpg::events
