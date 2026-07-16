#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::map {

struct WorldMarker { std::string id; std::string label; int32_t x=0; int32_t y=0; };
struct WorldMapNode {
    std::string id; std::string label;
    std::vector<WorldMarker> entrances, exits, checkpoints, spawn_points;
};
struct WorldRoute {
    std::string id, label, source_map_id, source_exit_id, target_map_id, target_entrance_id;
    std::string condition_key;
};
struct WorldGraphDiagnostic { std::string code, map_id, route_id, object_id, message; };
struct WorldTransferResult { bool success=false; std::string code, map_id, entrance_id; int32_t x=0; int32_t y=0; };
struct WorldGraphImpactReference { std::string route_id, route_label, role, other_map_id; };
struct WorldGraphImpact {
    bool object_found=false; bool rename_allowed=false; std::string code, map_id, marker_kind, marker_id, replacement_id;
    std::vector<WorldGraphImpactReference> affected_routes;
};
struct WorldGraphPreviewNode { std::string map_id, label; int32_t x=0, y=0; bool orphan=false; };
struct WorldGraphPreviewEdge { std::string route_id, source_map_id, target_map_id, condition_key; };
struct WorldGraphPreview { std::vector<WorldGraphPreviewNode> nodes; std::vector<WorldGraphPreviewEdge> edges; };

class ProjectWorldGraph {
  public:
    bool addMap(WorldMapNode map);
    bool addRoute(WorldRoute route);
    std::vector<WorldGraphDiagnostic> validate() const;
    WorldTransferResult transfer(const std::string& current_map_id, const std::string& exit_id,
                                 const std::map<std::string,bool>& conditions = {}) const;
    WorldGraphImpact previewMarkerChange(const std::string& map_id, const std::string& marker_kind,
                                         const std::string& marker_id, const std::string& replacement_id = {}) const;
    bool renameMarker(const WorldGraphImpact& reviewed_impact);
    WorldGraphPreview buildPreview() const;
    nlohmann::json toJson() const;
    static std::optional<ProjectWorldGraph> fromJson(const nlohmann::json& value);
    const std::vector<WorldMapNode>& maps() const { return maps_; }
    const std::vector<WorldRoute>& routes() const { return routes_; }
  private:
    std::vector<WorldMapNode> maps_;
    std::vector<WorldRoute> routes_;
};

} // namespace urpg::map
