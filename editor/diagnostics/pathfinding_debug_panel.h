#pragma once

#include "engine/core/level/path_request_router.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::editor {

struct PathfindingDebugNodeRow {
    size_t index = 0;
    int32_t x = 0;
    int32_t y = 0;
};

struct PathfindingDebugDiagnosticRow {
    std::string code;
    int32_t x = 0;
    int32_t y = 0;
    std::string reason;
};

struct PathfindingDebugSnapshot {
    bool has_route = false;
    std::string request_id;
    std::string actor_id;
    std::string surface_id;
    std::string source;
    std::string status;
    int32_t total_cost = 0;
    size_t node_count = 0;
    std::vector<PathfindingDebugNodeRow> node_rows;
    std::vector<PathfindingDebugDiagnosticRow> diagnostic_rows;

    [[nodiscard]] nlohmann::json toJson() const;
};

class PathfindingDebugPanel {
  public:
    void clear();
    void bindRoute(const urpg::level::RoutedPathRequest& route);

    [[nodiscard]] const PathfindingDebugSnapshot& snapshot() const { return snapshot_; }

  private:
    PathfindingDebugSnapshot snapshot_;
};

} // namespace urpg::editor
