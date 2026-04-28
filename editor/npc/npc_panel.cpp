#include "editor/npc/npc_panel.h"

#include <utility>

namespace urpg::editor::npc {

std::string NpcPanel::snapshot(const urpg::npc::NpcResolvedState& state) {
    return state.usedFallback ? "npc:fallback:" + state.mapId : "npc:routine:" + state.mapId;
}

void NpcSchedulePanel::loadDocument(urpg::npc::NpcScheduleDocument document) {
    document_ = std::move(document);
    if (npc_id_.empty() && !document_.routines.empty()) {
        npc_id_ = document_.routines.front().npc_id;
    }
    refresh();
}

void NpcSchedulePanel::setPreviewContext(std::string npc_id, int hour, std::set<std::string> available_maps) {
    npc_id_ = std::move(npc_id);
    hour_ = hour;
    available_maps_ = std::move(available_maps);
    refresh();
}

void NpcSchedulePanel::render() {
    refresh();
}

nlohmann::json NpcSchedulePanel::saveProjectData() const {
    return document_.toJson();
}

void NpcSchedulePanel::refresh() {
    preview_ = document_.preview(npc_id_, hour_, available_maps_);
    snapshot_.npc_id = npc_id_;
    snapshot_.hour = hour_;
    snapshot_.map_id = preview_.state.mapId;
    snapshot_.used_fallback = preview_.state.usedFallback;
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::npc
