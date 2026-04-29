#include "engine/core/world/world_map_graph.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace urpg::world {

void WorldMapGraph::addNode(WorldNode node) {
    nodes_.push_back(std::move(node));
}

void WorldMapGraph::addRoute(WorldRoute route) {
    routes_.push_back(std::move(route));
}

void WorldMapGraph::addFastTravelDestination(FastTravelDestination destination) {
    fast_travel_destinations_.push_back(std::move(destination));
}

void WorldMapGraph::addVehicleProfile(VehicleProfile profile) {
    vehicle_profiles_.push_back(std::move(profile));
}

bool WorldMapGraph::isRouteAvailable(const std::string& from,
                                      const std::string& to,
                                      const std::set<std::string>& flags,
                                      const std::set<std::string>& vehicles) const {
    return std::ranges::any_of(routes_, [&](const WorldRoute& route) {
        const bool flag_ok = route.required_flag.empty() || flags.contains(route.required_flag);
        const bool vehicle_ok = route.required_vehicle.empty() || vehicles.contains(route.required_vehicle);
        return route.from == from && route.to == to && flag_ok && vehicle_ok;
    });
}

FastTravelPreview WorldMapGraph::previewFastTravel(const std::string& current_node_id,
                                                   const std::string& destination_id,
                                                   const std::set<std::string>& flags) const {
    FastTravelPreview preview;
    preview.destination_id = destination_id;

    const auto destination = std::ranges::find_if(fast_travel_destinations_, [&](const FastTravelDestination& entry) {
        return entry.id == destination_id;
    });
    if (destination == fast_travel_destinations_.end()) {
        preview.diagnostics.push_back({"missing_fast_travel_destination", "fast travel destination does not exist"});
        return preview;
    }

    preview.preview_asset = destination->preview_asset;
    preview.locked_common_event = destination->locked_common_event;
    if (destination->hidden_when_current && destination->node_id == current_node_id) {
        preview.hidden_current_location = true;
        preview.diagnostics.push_back({"fast_travel_current_location_hidden",
                                       "fast travel destination is hidden because it is the current location"});
        return preview;
    }
    if (!destination->unlock_flag.empty() && !flags.contains(destination->unlock_flag)) {
        preview.diagnostics.push_back({"fast_travel_locked", "fast travel destination unlock flag is missing"});
        return preview;
    }

    std::ostringstream command;
    command << "transfer:" << destination->map_id << ':' << destination->x << ':' << destination->y;
    preview.command = command.str();
    preview.available = true;
    return preview;
}

VehiclePreview WorldMapGraph::previewVehicle(const std::string& vehicle_id, int terrain_tag) const {
    VehiclePreview preview;
    preview.vehicle_id = vehicle_id;

    const auto profile = std::ranges::find_if(vehicle_profiles_, [&](const VehicleProfile& entry) {
        return entry.id == vehicle_id;
    });
    if (profile == vehicle_profiles_.end()) {
        preview.diagnostics.push_back({"missing_vehicle_profile", "vehicle profile does not exist"});
        return preview;
    }

    if (!profile->allowed_terrain_tags.empty() &&
        std::ranges::find(profile->allowed_terrain_tags, terrain_tag) == profile->allowed_terrain_tags.end()) {
        preview.diagnostics.push_back({"vehicle_terrain_blocked", "vehicle profile does not allow this terrain tag"});
        return preview;
    }

    std::ostringstream command;
    command << "vehicle:" << profile->id << ":speed:" << profile->speed_multiplier
            << ":encounters:" << profile->encounter_rate_multiplier;
    if (!profile->transition_color.empty() || profile->transition_frames > 0) {
        command << ":transition:" << profile->transition_color << ':' << profile->transition_frames;
    }
    preview.command = command.str();

    if (!profile->bgm_asset.empty() || !profile->bgs_asset.empty()) {
        preview.audio_command = "vehicle_audio:" + profile->bgm_asset + ':' + profile->bgs_asset;
    }
    if (profile->interior_map_id > 0) {
        std::ostringstream transfer;
        transfer << "vehicle_interior:" << profile->interior_map_id << ':' << profile->interior_x << ':'
                 << profile->interior_y;
        preview.interior_transfer_command = transfer.str();
    }

    preview.available = true;
    return preview;
}

std::vector<WorldDiagnostic> WorldMapGraph::validate() const {
    std::vector<WorldDiagnostic> diagnostics;
    const auto node_exists = [&](const std::string& id) {
        return std::ranges::any_of(nodes_, [&](const WorldNode& node) { return node.id == id; });
    };
    for (const auto& route : routes_) {
        if (route.from == route.to && route.required_flag == route.to) {
            diagnostics.push_back({"route_unlocks_itself", "travel route unlock condition points at its own destination"});
        }
        if (!node_exists(route.from) || !node_exists(route.to)) {
            diagnostics.push_back({"missing_route_node", "travel route references a missing world node"});
        }
    }
    for (const auto& destination : fast_travel_destinations_) {
        if (destination.id.empty() || destination.node_id.empty() || destination.map_id <= 0) {
            diagnostics.push_back({"invalid_fast_travel_destination",
                                   "fast travel destination needs an id, node id, and positive map id"});
        }
        if (!destination.node_id.empty() && !node_exists(destination.node_id)) {
            diagnostics.push_back({"missing_fast_travel_node",
                                   "fast travel destination references a missing world node"});
        }
    }
    for (const auto& profile : vehicle_profiles_) {
        if (profile.id.empty()) {
            diagnostics.push_back({"invalid_vehicle_profile", "vehicle profile needs an id"});
        }
        if (profile.speed_multiplier <= 0.0) {
            diagnostics.push_back({"invalid_vehicle_speed", "vehicle speed multiplier must be positive"});
        }
        if (profile.encounter_rate_multiplier < 0.0) {
            diagnostics.push_back({"invalid_vehicle_encounter_rate",
                                   "vehicle encounter rate multiplier cannot be negative"});
        }
        if (profile.interior_map_id < 0) {
            diagnostics.push_back({"invalid_vehicle_interior", "vehicle interior map id cannot be negative"});
        }
    }
    return diagnostics;
}

} // namespace urpg::world
