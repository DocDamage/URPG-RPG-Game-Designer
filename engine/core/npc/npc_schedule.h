#pragma once

#include <set>
#include <string>
#include <vector>

namespace urpg::npc {

struct GridPosition {
    int x{0};
    int y{0};
};

struct NpcRoutine {
    std::string npc_id;
    std::string map_id;
    int start_hour{0};
    int end_hour{23};
    GridPosition position;
    std::string animation;
    std::string dialogue_state;
};

struct NpcFallback {
    std::string map_id;
    GridPosition position;
    std::string animation;
    std::string dialogue_state;
};

struct NpcResolvedState {
    bool usedFallback{false};
    std::string mapId;
    GridPosition position;
    std::string animation;
    std::string dialogueState;
};

class NpcSchedule {
public:
    void addRoutine(NpcRoutine routine);
    void setFallback(NpcFallback fallback);
    [[nodiscard]] NpcResolvedState resolve(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const;

private:
    std::vector<NpcRoutine> routines_;
    NpcFallback fallback_;
};

} // namespace urpg::npc
