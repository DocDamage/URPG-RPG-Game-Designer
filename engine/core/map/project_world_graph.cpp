#include "engine/core/map/project_world_graph.h"

#include <algorithm>
#include <set>
#include <tuple>

namespace urpg::map {
namespace {
const WorldMarker* marker(const std::vector<WorldMarker>& values, const std::string& id) {
    const auto found=std::find_if(values.begin(),values.end(),[&](const auto& value){return value.id==id;});
    return found==values.end()?nullptr:&*found;
}
bool validMarkers(const std::vector<WorldMarker>& values) { std::set<std::string> ids; return std::all_of(values.begin(),values.end(),[&](const auto& value){return !value.id.empty()&&ids.insert(value.id).second;}); }
nlohmann::json markers(const std::vector<WorldMarker>& values){ auto result=nlohmann::json::array(); for(const auto& value:values) result.push_back({{"id",value.id},{"label",value.label},{"x",value.x},{"y",value.y}}); return result; }
std::vector<WorldMarker> readMarkers(const nlohmann::json& value){ std::vector<WorldMarker> result; if(!value.is_array()) return result; for(const auto& row:value) if(row.is_object()) result.push_back({row.value("id",""),row.value("label",""),row.value("x",0),row.value("y",0)}); return result; }
}

bool ProjectWorldGraph::addMap(WorldMapNode map) {
    if(map.id.empty()||map.label.empty()||!validMarkers(map.entrances)||!validMarkers(map.exits)||!validMarkers(map.checkpoints)||!validMarkers(map.spawn_points)||
       std::any_of(maps_.begin(),maps_.end(),[&](const auto& existing){return existing.id==map.id;})) return false;
    maps_.push_back(std::move(map)); std::sort(maps_.begin(),maps_.end(),[](const auto& a,const auto& b){return a.id<b.id;}); return true;
}

bool ProjectWorldGraph::updateMap(WorldMapNode map) {
    if (map.id.empty() || map.label.empty() || !validMarkers(map.entrances) || !validMarkers(map.exits) ||
        !validMarkers(map.checkpoints) || !validMarkers(map.spawn_points)) return false;
    const auto found = std::find_if(maps_.begin(), maps_.end(), [&](const auto& item) { return item.id == map.id; });
    if (found == maps_.end()) return false;
    *found = std::move(map);
    return true;
}
bool ProjectWorldGraph::addRoute(WorldRoute route) {
    if(route.id.empty()||route.label.empty()||route.source_map_id.empty()||route.source_exit_id.empty()||route.target_map_id.empty()||route.target_entrance_id.empty()||
       std::any_of(routes_.begin(),routes_.end(),[&](const auto& existing){return existing.id==route.id;})) return false;
    routes_.push_back(std::move(route)); std::sort(routes_.begin(),routes_.end(),[](const auto& a,const auto& b){return a.id<b.id;}); return true;
}
std::vector<WorldGraphDiagnostic> ProjectWorldGraph::validate() const {
    std::vector<WorldGraphDiagnostic> diagnostics;
    const auto findMap=[&](const std::string& id)->const WorldMapNode*{const auto found=std::find_if(maps_.begin(),maps_.end(),[&](const auto& map){return map.id==id;}); return found==maps_.end()?nullptr:&*found;};
    std::set<std::string> connected;
    for(const auto& route:routes_){ const auto* source=findMap(route.source_map_id); const auto* target=findMap(route.target_map_id);
        if(source==nullptr) diagnostics.push_back({"route_source_map_missing",route.source_map_id,route.id,route.source_exit_id,"Route source map is missing."});
        else if(marker(source->exits,route.source_exit_id)==nullptr) diagnostics.push_back({"route_source_exit_missing",source->id,route.id,route.source_exit_id,"Route source exit is missing."});
        if(target==nullptr) diagnostics.push_back({"route_target_map_missing",route.target_map_id,route.id,route.target_entrance_id,"Route target map is missing."});
        else if(marker(target->entrances,route.target_entrance_id)==nullptr) diagnostics.push_back({"route_target_entrance_missing",target->id,route.id,route.target_entrance_id,"Route target entrance is missing."});
        if(source&&target&&marker(source->exits,route.source_exit_id)&&marker(target->entrances,route.target_entrance_id)){connected.insert(source->id);connected.insert(target->id);}
    }
    if(maps_.size()>1) for(const auto& map:maps_) if(!connected.contains(map.id)) diagnostics.push_back({"orphan_map",map.id,"",map.id,"Map has no valid incoming or outgoing route."});
    std::sort(diagnostics.begin(),diagnostics.end(),[](const auto& a,const auto& b){return std::tie(a.map_id,a.route_id,a.code)<std::tie(b.map_id,b.route_id,b.code);}); return diagnostics;
}
WorldTransferResult ProjectWorldGraph::transfer(const std::string& current,const std::string& exit,const std::map<std::string,bool>& conditions) const {
    const auto route=std::find_if(routes_.begin(),routes_.end(),[&](const auto& candidate){return candidate.source_map_id==current&&candidate.source_exit_id==exit&&
        (candidate.condition_key.empty()||(conditions.contains(candidate.condition_key)&&conditions.at(candidate.condition_key)));});
    if(route==routes_.end()) return {false,"world_route_unavailable",current,"",0,0};
    const auto target=std::find_if(maps_.begin(),maps_.end(),[&](const auto& map){return map.id==route->target_map_id;});
    if(target==maps_.end()) return {false,"world_route_target_missing",current,"",0,0};
    const auto* entrance=marker(target->entrances,route->target_entrance_id);
    if(!entrance) return {false,"world_route_entrance_missing",current,"",0,0};
    return {true,"world_route_transferred",target->id,entrance->id,entrance->x,entrance->y};
}
WorldGraphImpact ProjectWorldGraph::previewMarkerChange(const std::string& mapId,const std::string& kind,
                                                        const std::string& markerId,const std::string& replacementId) const {
    WorldGraphImpact result; result.map_id=mapId;result.marker_kind=kind;result.marker_id=markerId;result.replacement_id=replacementId;
    const auto map=std::find_if(maps_.begin(),maps_.end(),[&](const auto& value){return value.id==mapId;});
    if(map==maps_.end()){result.code="world_marker_map_missing";return result;}
    const std::vector<WorldMarker>* values=kind=="entrance"?&map->entrances:kind=="exit"?&map->exits:kind=="checkpoint"?&map->checkpoints:kind=="spawn"?&map->spawn_points:nullptr;
    if(values==nullptr){result.code="world_marker_kind_invalid";return result;} result.object_found=marker(*values,markerId)!=nullptr;
    if(!result.object_found){result.code="world_marker_missing";return result;}
    for(const auto& route:routes_){if(kind=="entrance"&&route.target_map_id==mapId&&route.target_entrance_id==markerId)result.affected_routes.push_back({route.id,route.label,"target_entrance",route.source_map_id});
        if(kind=="exit"&&route.source_map_id==mapId&&route.source_exit_id==markerId)result.affected_routes.push_back({route.id,route.label,"source_exit",route.target_map_id});}
    std::sort(result.affected_routes.begin(),result.affected_routes.end(),[](const auto& a,const auto& b){return a.route_id<b.route_id;});
    result.rename_allowed=!replacementId.empty()&&marker(*values,replacementId)==nullptr; result.code=replacementId.empty()?"world_marker_delete_impact_ready":
        (result.rename_allowed?"world_marker_rename_impact_ready":"world_marker_replacement_duplicate"); return result;
}
bool ProjectWorldGraph::renameMarker(const WorldGraphImpact& reviewed) {
    if(!reviewed.object_found||!reviewed.rename_allowed||reviewed.replacement_id.empty())return false;
    auto map=std::find_if(maps_.begin(),maps_.end(),[&](const auto& value){return value.id==reviewed.map_id;});if(map==maps_.end())return false;
    std::vector<WorldMarker>* values=reviewed.marker_kind=="entrance"?&map->entrances:reviewed.marker_kind=="exit"?&map->exits:reviewed.marker_kind=="checkpoint"?&map->checkpoints:reviewed.marker_kind=="spawn"?&map->spawn_points:nullptr;
    if(values==nullptr)return false;
    auto item=std::find_if(values->begin(),values->end(),[&](const auto& value){return value.id==reviewed.marker_id;});
    if(item==values->end()||marker(*values,reviewed.replacement_id))return false;
    item->id=reviewed.replacement_id;
    for(auto& route:routes_){if(reviewed.marker_kind=="entrance"&&route.target_map_id==reviewed.map_id&&route.target_entrance_id==reviewed.marker_id)route.target_entrance_id=reviewed.replacement_id;
        if(reviewed.marker_kind=="exit"&&route.source_map_id==reviewed.map_id&&route.source_exit_id==reviewed.marker_id)route.source_exit_id=reviewed.replacement_id;}return true;
}
WorldGraphPreview ProjectWorldGraph::buildPreview() const {WorldGraphPreview preview;std::set<std::string> orphan;
    for(const auto& diagnostic:validate())if(diagnostic.code=="orphan_map")orphan.insert(diagnostic.map_id);
    for(size_t index=0;index<maps_.size();++index)preview.nodes.push_back({maps_[index].id,maps_[index].label,static_cast<int32_t>(index%4)*240,static_cast<int32_t>(index/4)*160,orphan.contains(maps_[index].id)});
    for(const auto& route:routes_)preview.edges.push_back({route.id,route.source_map_id,route.target_map_id,route.condition_key});
    return preview;
}
nlohmann::json ProjectWorldGraph::toJson() const { auto maps=nlohmann::json::array(), routes=nlohmann::json::array();
    for(const auto& map:maps_) maps.push_back({{"id",map.id},{"label",map.label},{"entrances",markers(map.entrances)},{"exits",markers(map.exits)},{"checkpoints",markers(map.checkpoints)},{"spawn_points",markers(map.spawn_points)}});
    for(const auto& route:routes_) routes.push_back({{"id",route.id},{"label",route.label},{"source_map_id",route.source_map_id},{"source_exit_id",route.source_exit_id},{"target_map_id",route.target_map_id},{"target_entrance_id",route.target_entrance_id},{"condition_key",route.condition_key}});
    return {{"schema","urpg/project_world_graph/v1"},{"maps",std::move(maps)},{"routes",std::move(routes)}}; }
std::optional<ProjectWorldGraph> ProjectWorldGraph::fromJson(const nlohmann::json& value){ if(!value.is_object()||value.value("schema","")!="urpg/project_world_graph/v1"||!value.value("maps",nlohmann::json::array()).is_array()||!value.value("routes",nlohmann::json::array()).is_array()) return std::nullopt;
    ProjectWorldGraph graph; for(const auto& row:value["maps"]){ WorldMapNode map; map.id=row.value("id","");map.label=row.value("label","");map.entrances=readMarkers(row.value("entrances",nlohmann::json::array()));map.exits=readMarkers(row.value("exits",nlohmann::json::array()));map.checkpoints=readMarkers(row.value("checkpoints",nlohmann::json::array()));map.spawn_points=readMarkers(row.value("spawn_points",nlohmann::json::array()));if(!graph.addMap(std::move(map)))return std::nullopt;}
    for(const auto& row:value["routes"]){if(!graph.addRoute({row.value("id",""),row.value("label",""),row.value("source_map_id",""),row.value("source_exit_id",""),row.value("target_map_id",""),row.value("target_entrance_id",""),row.value("condition_key","")}))return std::nullopt;} return graph; }

} // namespace urpg::map
