#include "engine/core/level/path_request_router.h"

namespace urpg::level {

std::string pathRequestSourceName(PathRequestSource source) {
    switch (source) {
    case PathRequestSource::MapRuntime:
        return "map_runtime";
    case PathRequestSource::EventRuntime:
        return "event_runtime";
    case PathRequestSource::EditorDiagnostics:
        return "editor_diagnostics";
    }
    return "map_runtime";
}

nlohmann::json RoutedPathRequest::toJson() const {
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& node : path.nodes) {
        nodes.push_back({{"x", node.x}, {"y", node.y}});
    }

    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : path.diagnostics) {
        diagnostics.push_back({{"code", diagnostic.code},
                               {"x", diagnostic.point.x},
                               {"y", diagnostic.point.y},
                               {"reason", diagnostic.reason}});
    }

    return {
        {"request_id", request_id},
        {"actor_id", actor_id},
        {"surface_id", surface_id},
        {"source", pathRequestSourceName(source)},
        {"status", status},
        {"route",
         {{"found", path.found},
          {"reason", path.reason},
          {"total_cost", path.total_cost},
          {"node_count", nodes.size()},
          {"nodes", nodes}}},
        {"diagnostics", diagnostics},
    };
}

RoutedPathRequest RoutePathRequest(const PathfindingGraph& graph, const PathRequest& request) {
    RoutedPathRequest routed;
    routed.request_id = request.request_id;
    routed.actor_id = request.actor_id;
    routed.surface_id = request.surface_id;
    routed.source = request.source;
    routed.path = graph.findPath(request.start, request.goal);
    routed.ok = routed.path.found;
    routed.status = routed.ok ? "routed" : "blocked";
    return routed;
}

} // namespace urpg::level
