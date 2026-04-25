#pragma once

#include "engine/core/world/world_map_graph.h"

#include <string>

namespace urpg::editor::world {

class WorldPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::world::WorldMapGraph& graph);
};

} // namespace urpg::editor::world
