#include "editor/diagnostics/pathfinding_debug_panel.h"

namespace urpg::editor {

nlohmann::json PathfindingDebugSnapshot::toJson() const {
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& row : node_rows) {
        nodes.push_back({{"index", row.index}, {"x", row.x}, {"y", row.y}});
    }

    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& row : diagnostic_rows) {
        diagnostics.push_back({{"code", row.code}, {"x", row.x}, {"y", row.y}, {"reason", row.reason}});
    }

    return {
        {"has_route", has_route}, {"request_id", request_id},   {"actor_id", actor_id},     {"surface_id", surface_id},
        {"source", source},       {"status", status},           {"total_cost", total_cost}, {"node_count", node_count},
        {"nodes", nodes},         {"diagnostics", diagnostics},
    };
}

void PathfindingDebugPanel::clear() {
    snapshot_ = {};
}

void PathfindingDebugPanel::bindRoute(const urpg::level::RoutedPathRequest& route) {
    snapshot_ = {};
    snapshot_.has_route = route.ok;
    snapshot_.request_id = route.request_id;
    snapshot_.actor_id = route.actor_id;
    snapshot_.surface_id = route.surface_id;
    snapshot_.source = urpg::level::pathRequestSourceName(route.source);
    snapshot_.status = route.status;
    snapshot_.total_cost = route.path.total_cost;
    snapshot_.node_count = route.path.nodes.size();

    for (size_t i = 0; i < route.path.nodes.size(); ++i) {
        const auto& node = route.path.nodes[i];
        snapshot_.node_rows.push_back({i, node.x, node.y});
    }

    for (const auto& diagnostic : route.path.diagnostics) {
        snapshot_.diagnostic_rows.push_back(
            {diagnostic.code, diagnostic.point.x, diagnostic.point.y, diagnostic.reason});
    }
}

} // namespace urpg::editor
