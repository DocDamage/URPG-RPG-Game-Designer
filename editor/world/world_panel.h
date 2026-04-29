#pragma once

#include "engine/core/world/world_map_graph.h"

#include <string>

namespace urpg::editor::world {

class WorldPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::world::WorldMapGraph& graph);
    [[nodiscard]] static std::string fastTravelSnapshot(const urpg::world::FastTravelPreview& preview);
    [[nodiscard]] static std::string vehicleSnapshot(const urpg::world::VehiclePreview& preview);
};

} // namespace urpg::editor::world
