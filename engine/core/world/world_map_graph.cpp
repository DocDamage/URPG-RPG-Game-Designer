#include "engine/core/world/world_map_graph.h"

#include <algorithm>

namespace urpg::world {

void WorldMapGraph::addNode(WorldNode node) {
    nodes_.push_back(std::move(node));
}

void WorldMapGraph::addRoute(WorldRoute route) {
    routes_.push_back(std::move(route));
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

std::vector<WorldDiagnostic> WorldMapGraph::validate() const {
    std::vector<WorldDiagnostic> diagnostics;
    for (const auto& route : routes_) {
        if (route.from == route.to && route.required_flag == route.to) {
            diagnostics.push_back({"route_unlocks_itself", "travel route unlock condition points at its own destination"});
        }
        const auto node_exists = [&](const std::string& id) {
            return std::ranges::any_of(nodes_, [&](const WorldNode& node) { return node.id == id; });
        };
        if (!node_exists(route.from) || !node_exists(route.to)) {
            diagnostics.push_back({"missing_route_node", "travel route references a missing world node"});
        }
    }
    return diagnostics;
}

} // namespace urpg::world
