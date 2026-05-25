#pragma once

#include "engine/core/level/pathfinding_graph.h"

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::level {

enum class PathRequestSource {
    MapRuntime,
    EventRuntime,
    EditorDiagnostics,
};

struct PathRequest {
    std::string request_id;
    std::string actor_id;
    std::string surface_id;
    PathRequestSource source = PathRequestSource::MapRuntime;
    PathGridPoint start;
    PathGridPoint goal;
};

struct RoutedPathRequest {
    bool ok = false;
    std::string request_id;
    std::string actor_id;
    std::string surface_id;
    PathRequestSource source = PathRequestSource::MapRuntime;
    std::string status;
    PathfindingResult path;

    [[nodiscard]] nlohmann::json toJson() const;
};

[[nodiscard]] std::string pathRequestSourceName(PathRequestSource source);
[[nodiscard]] RoutedPathRequest RoutePathRequest(const PathfindingGraph& graph, const PathRequest& request);

} // namespace urpg::level
