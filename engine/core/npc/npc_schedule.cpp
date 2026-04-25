#include "engine/core/npc/npc_schedule.h"

namespace urpg::npc {

void NpcSchedule::addRoutine(NpcRoutine routine) {
    routines_.push_back(std::move(routine));
}

void NpcSchedule::setFallback(NpcFallback fallback) {
    fallback_ = std::move(fallback);
}

NpcResolvedState NpcSchedule::resolve(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const {
    for (const auto& routine : routines_) {
        if (routine.npc_id == npc_id && hour >= routine.start_hour && hour <= routine.end_hour && available_maps.contains(routine.map_id)) {
            return {false, routine.map_id, routine.position, routine.animation, routine.dialogue_state};
        }
    }
    return {true, fallback_.map_id, fallback_.position, fallback_.animation, fallback_.dialogue_state};
}

} // namespace urpg::npc
