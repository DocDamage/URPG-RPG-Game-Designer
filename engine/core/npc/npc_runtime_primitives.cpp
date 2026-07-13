#include "engine/core/npc/npc_runtime_primitives.h"

#include <algorithm>
#include <cstdlib>

namespace urpg::npc {

namespace {
constexpr size_t kMaxNpcTextLength = 160;
}

bool NpcRuntimePrimitives::registerNpc(NpcRuntimeState state) {
    if (state.npcId.empty() || state.mapId.empty()) {
        return false;
    }
    return npcs_.emplace(state.npcId, std::move(state)).second;
}

NpcRuntimeResult NpcRuntimePrimitives::missingNpc(const std::string& npcId) const {
    return {false, "npc_not_found", "No authored NPC exists with id '" + npcId + "'.", {}, {}};
}

NpcRuntimeResult NpcRuntimePrimitives::moveTo(const std::string& npcId, const level::PathfindingGraph& graph,
                                               const level::PathGridPoint goal) {
    auto found = npcs_.find(npcId);
    if (found == npcs_.end()) return missingNpc(npcId);
    const level::PathRequest request{npcId + ":move", npcId, found->second.mapId, level::PathRequestSource::EventRuntime,
                                     found->second.position, goal};
    const auto routed = level::RoutePathRequest(graph, request);
    if (!routed.ok) {
        return {false, "npc_path_rejected", routed.status, found->second, routed.path.nodes};
    }
    found->second.position = goal;
    found->second.moving = false;
    found->second.activity = "move_complete";
    return {true, "npc_move_complete", "NPC moved through the deterministic path router.", found->second, routed.path.nodes};
}

NpcRuntimeResult NpcRuntimePrimitives::stop(const std::string& npcId) {
    auto found = npcs_.find(npcId);
    if (found == npcs_.end()) return missingNpc(npcId);
    found->second.moving = false;
    found->second.activity = "stopped";
    return {true, "npc_stopped", "NPC movement stopped.", found->second, {}};
}

NpcRuntimeResult NpcRuntimePrimitives::face(const std::string& npcId, NpcFacing facing) {
    auto found = npcs_.find(npcId);
    if (found == npcs_.end()) return missingNpc(npcId);
    found->second.facing = facing;
    return {true, "npc_facing_changed", "NPC facing updated.", found->second, {}};
}

NpcRuntimeResult NpcRuntimePrimitives::setActivity(const std::string& npcId, std::string activity) {
    auto found = npcs_.find(npcId);
    if (found == npcs_.end()) return missingNpc(npcId);
    found->second.activity = std::move(activity);
    return {true, "npc_activity_changed", "NPC activity updated.", found->second, {}};
}

NpcRuntimeResult NpcRuntimePrimitives::setThought(const std::string& npcId, std::string thought) {
    auto found = npcs_.find(npcId);
    if (found == npcs_.end()) return missingNpc(npcId);
    if (thought.size() > kMaxNpcTextLength) {
        return {false, "npc_thought_too_long", "NPC thought/bark text exceeds the bounded authored limit.", found->second, {}};
    }
    found->second.thought = std::move(thought);
    return {true, "npc_thought_changed", "NPC thought/bark updated.", found->second, {}};
}

NpcRuntimeResult NpcRuntimePrimitives::observeProximity(const std::string& npcId, level::PathGridPoint player,
                                                         int maxDistance) const {
    const auto* state = find(npcId);
    if (state == nullptr) return missingNpc(npcId);
    const int distance = std::abs(state->position.x - player.x) + std::abs(state->position.y - player.y);
    if (distance > std::max(0, maxDistance)) {
        return {false, "npc_player_out_of_range", "Player is outside the authored interaction distance.", *state, {}};
    }
    return {true, "npc_player_in_range", "Player is inside the authored interaction distance.", *state, {}};
}

const NpcRuntimeState* NpcRuntimePrimitives::find(const std::string& npcId) const {
    const auto found = npcs_.find(npcId);
    return found == npcs_.end() ? nullptr : &found->second;
}

} // namespace urpg::npc
