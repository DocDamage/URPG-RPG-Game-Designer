#include "editor/world/world_panel.h"

namespace urpg::editor::world {

std::string WorldPanel::snapshot(const urpg::world::WorldMapGraph& graph) {
    return graph.validate().empty() ? "world:ready" : "world:diagnostics";
}

} // namespace urpg::editor::world
