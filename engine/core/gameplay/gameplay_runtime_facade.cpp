#include "engine/core/gameplay/gameplay_runtime_facade.h"

namespace urpg::gameplay {

void GameplayRuntimeFacade::bindMap(const map::GridPartDocument* document, const level::PathfindingGraph* graph) {
    document_ = document;
    graph_ = graph;
}

bool GameplayRuntimeFacade::registerNpc(npc::NpcRuntimeState state) { return npcs_.registerNpc(std::move(state)); }

GameplayFacadeResult GameplayRuntimeFacade::queryMapAt(int32_t x, int32_t y) const {
    if (document_ == nullptr) return {false, "gameplay_map_unbound", "No map document is bound to the gameplay facade.", {}, {}};
    if (!document_->inBounds(x, y)) return {false, "gameplay_map_point_out_of_bounds", "The requested map point is outside the active map.", {}, {}};
    GameplayFacadeResult result{true, "gameplay_map_query_ok", "Map query completed.", {}, {}};
    for (const auto* part : document_->partsAt(x, y)) result.objectIds.push_back(part->instance_id);
    return result;
}

GameplayFacadeResult GameplayRuntimeFacade::moveNpc(const std::string& npcId, level::PathGridPoint goal) {
    if (graph_ == nullptr) return {false, "gameplay_path_graph_unbound", "No pathfinding graph is bound to the gameplay facade.", {}, {}};
    const auto result = npcs_.moveTo(npcId, *graph_, goal);
    return {result.success, result.code, result.message, {}, result.path};
}

GameplayFacadeResult GameplayRuntimeFacade::invokeEvent(EventInvocation invocation) {
    if (invocation.event_id.empty()) return {false, "gameplay_event_missing_id", "An event id is required.", {}, {}};
    if (!events_.CanEnter(invocation)) return {false, "gameplay_event_reentrant", "The event cannot enter at its current reentrancy depth.", {}, {}};
    events_.BeginInvocation(invocation);
    events_.EndInvocation(invocation);
    return {true, "gameplay_event_invoked", "Event invocation completed through the native event runtime.", {}, {}};
}

GameplayFacadeResult GameplayRuntimeFacade::bindInput(std::string actionId, std::string commandId) {
    if (actionId.empty() || commandId.empty()) return {false, "gameplay_input_binding_invalid", "Input action and command ids are required.", {}, {}};
    inputBindings_[std::move(actionId)] = std::move(commandId);
    return {true, "gameplay_input_bound", "Input binding registered.", {}, {}};
}

GameplayFacadeResult GameplayRuntimeFacade::addResource(std::string resourceId, int delta) {
    if (resourceId.empty()) return {false, "gameplay_resource_missing_id", "A resource id is required.", {}, {}};
    resources_[resourceId] += delta;
    return {true, "gameplay_resource_changed", "Resource total changed.", {}, {}};
}

int GameplayRuntimeFacade::resourceCount(const std::string& resourceId) const {
    const auto found = resources_.find(resourceId);
    return found == resources_.end() ? 0 : found->second;
}

} // namespace urpg::gameplay
