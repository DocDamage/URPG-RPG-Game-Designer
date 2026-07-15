#pragma once

#include "engine/core/level/path_request_router.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::npc {

enum class NpcFacing { Down, Left, Right, Up };

struct NpcRuntimeState {
    std::string npcId;
    std::string mapId;
    level::PathGridPoint position;
    NpcFacing facing = NpcFacing::Down;
    bool moving = false;
    std::string activity;
    std::string thought;
};

struct NpcRuntimeResult {
    bool success = false;
    std::string code;
    std::string message;
    NpcRuntimeState state;
    std::vector<level::PathGridPoint> path;
};

class NpcRuntimePrimitives {
  public:
    bool registerNpc(NpcRuntimeState state);
    NpcRuntimeResult moveTo(const std::string& npcId, const level::PathfindingGraph& graph, level::PathGridPoint goal);
    NpcRuntimeResult stop(const std::string& npcId);
    NpcRuntimeResult face(const std::string& npcId, NpcFacing facing);
    NpcRuntimeResult setActivity(const std::string& npcId, std::string activity);
    NpcRuntimeResult setThought(const std::string& npcId, std::string thought);
    NpcRuntimeResult observeProximity(const std::string& npcId, level::PathGridPoint player, int maxDistance) const;
    const NpcRuntimeState* find(const std::string& npcId) const;

  private:
    NpcRuntimeResult missingNpc(const std::string& npcId) const;
    std::unordered_map<std::string, NpcRuntimeState> npcs_;
};

} // namespace urpg::npc
