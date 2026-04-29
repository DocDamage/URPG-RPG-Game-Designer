#include "editor/world/world_panel.h"

namespace urpg::editor::world {

std::string WorldPanel::snapshot(const urpg::world::WorldMapGraph& graph) {
    return graph.validate().empty() ? "world:ready" : "world:diagnostics";
}

std::string WorldPanel::fastTravelSnapshot(const urpg::world::FastTravelPreview& preview) {
    if (preview.hidden_current_location) {
        return "fast_travel:hidden_current:" + preview.destination_id;
    }
    if (!preview.available) {
        return "fast_travel:locked:" + preview.destination_id + ':' + preview.locked_common_event;
    }
    return "fast_travel:available:" + preview.destination_id + ':' + preview.command + ':' + preview.preview_asset;
}

std::string WorldPanel::vehicleSnapshot(const urpg::world::VehiclePreview& preview) {
    if (!preview.available) {
        return "vehicle:blocked:" + preview.vehicle_id;
    }
    return "vehicle:available:" + preview.vehicle_id + ':' + preview.command + ':' + preview.audio_command + ':' +
           preview.interior_transfer_command;
}

} // namespace urpg::editor::world
