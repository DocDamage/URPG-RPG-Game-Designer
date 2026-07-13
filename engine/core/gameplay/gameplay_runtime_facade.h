#pragma once

#include "engine/core/events/event_runtime.h"
#include "engine/core/level/pathfinding_graph.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/npc/npc_runtime_primitives.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::gameplay {

struct GameplayFacadeResult {
    bool success = false;
    std::string code;
    std::string message;
    std::vector<std::string> objectIds;
    std::vector<level::PathGridPoint> path;
};

// The sole gameplay-facing composition root for compact creator/runtime
// operations. It delegates to the existing map, event, and path owners rather
// than defining a parallel game state or persistence model.
class GameplayRuntimeFacade {
  public:
    void bindMap(const map::GridPartDocument* document, const level::PathfindingGraph* graph);
    bool registerNpc(npc::NpcRuntimeState state);
    GameplayFacadeResult queryMapAt(int32_t x, int32_t y) const;
    GameplayFacadeResult moveNpc(const std::string& npcId, level::PathGridPoint goal);
    GameplayFacadeResult invokeEvent(EventInvocation invocation);
    GameplayFacadeResult bindInput(std::string actionId, std::string commandId);
    GameplayFacadeResult addResource(std::string resourceId, int delta);
    int resourceCount(const std::string& resourceId) const;

  private:
    const map::GridPartDocument* document_ = nullptr;
    const level::PathfindingGraph* graph_ = nullptr;
    npc::NpcRuntimePrimitives npcs_;
    EventDispatchSession events_;
    std::unordered_map<std::string, std::string> inputBindings_;
    std::unordered_map<std::string, int> resources_;
};

} // namespace urpg::gameplay
