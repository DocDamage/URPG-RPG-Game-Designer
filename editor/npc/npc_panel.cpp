#include "editor/npc/npc_panel.h"

namespace urpg::editor::npc {

std::string NpcPanel::snapshot(const urpg::npc::NpcResolvedState& state) {
    return state.usedFallback ? "npc:fallback:" + state.mapId : "npc:routine:" + state.mapId;
}

} // namespace urpg::editor::npc
