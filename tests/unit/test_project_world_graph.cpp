#include "engine/core/map/project_world_graph.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

TEST_CASE("Project world graph authors and plays a conditional two-map transfer loop", "[map][world_graph]") {
    using namespace urpg::map; ProjectWorldGraph graph;
    REQUIRE(graph.addMap({"town","Town",{{"from_forest","Forest Gate",1,5}},{{"to_forest","Forest Road",8,5}},{{"town_saved","Town",4,4}},{{"town_start","Start",3,4}}}));
    REQUIRE(graph.addMap({"forest","Forest",{{"from_town","Town Trail",1,2}},{{"to_town","Town Trail",7,2}},{{"forest_saved","Forest",4,2}},{{"forest_start","Start",2,2}}}));
    REQUIRE(graph.addRoute({"town_forest","To Forest","town","to_forest","forest","from_town","forest_open"}));
    REQUIRE(graph.addRoute({"forest_town","To Town","forest","to_town","town","from_forest",""}));
    REQUIRE(graph.validate().empty());
    REQUIRE_FALSE(graph.transfer("town","to_forest").success);
    const auto outward=graph.transfer("town","to_forest",{{"forest_open",true}}); REQUIRE(outward.success); REQUIRE(outward.map_id=="forest"); REQUIRE(outward.x==1); REQUIRE(outward.y==2);
    const auto returned=graph.transfer(outward.map_id,"to_town"); REQUIRE(returned.success); REQUIRE(returned.map_id=="town"); REQUIRE(returned.entrance_id=="from_forest");
    const auto json=graph.toJson(); const auto restored=ProjectWorldGraph::fromJson(json); REQUIRE(restored.has_value()); REQUIRE(restored->toJson()==json); REQUIRE(restored->maps()[0].checkpoints.size()==1); REQUIRE(restored->maps()[0].spawn_points.size()==1);
}

TEST_CASE("Project world graph reports object-linked invalid routes and orphan maps", "[map][world_graph]") {
    using namespace urpg::map; ProjectWorldGraph graph;
    REQUIRE(graph.addMap({"orphan","Orphan",{}, {},{}, {}})); REQUIRE(graph.addMap({"town","Town",{},{{"north","North",1,1}}, {}, {}}));
    REQUIRE(graph.addRoute({"broken","Broken","town","missing","missing_map","entrance",""}));
    const auto diagnostics=graph.validate(); REQUIRE(diagnostics.size()==4);
    REQUIRE(std::count_if(diagnostics.begin(),diagnostics.end(),[](const auto& diagnostic){return diagnostic.code=="orphan_map";})==2);
    const auto source=std::find_if(diagnostics.begin(),diagnostics.end(),[](const auto& diagnostic){return diagnostic.code=="route_source_exit_missing";});
    REQUIRE(source!=diagnostics.end()); REQUIRE(source->route_id=="broken"); REQUIRE(source->object_id=="missing");
}

TEST_CASE("World preview and marker impact enumerate and rewrite every affected route", "[map][world_graph][impact]") {
    using namespace urpg::map; ProjectWorldGraph graph;
    REQUIRE(graph.addMap({"hub","Hub",{{"gate","Gate",1,1}},{{"road","Road",2,1}}, {}, {}}));
    REQUIRE(graph.addMap({"field","Field",{{"from_hub","Hub",1,1}},{{"to_hub","Hub",2,1}}, {}, {}}));
    REQUIRE(graph.addMap({"cave","Cave",{{"from_hub","Hub",1,1}},{{"to_hub","Hub",2,1}}, {}, {}}));
    REQUIRE(graph.addRoute({"field_hub","Field to Hub","field","to_hub","hub","gate",""}));
    REQUIRE(graph.addRoute({"cave_hub","Cave to Hub","cave","to_hub","hub","gate","key"}));
    const auto deletion=graph.previewMarkerChange("hub","entrance","gate"); REQUIRE(deletion.object_found); REQUIRE_FALSE(deletion.rename_allowed); REQUIRE(deletion.affected_routes.size()==2);
    REQUIRE(deletion.affected_routes[0].route_id=="cave_hub"); REQUIRE(deletion.affected_routes[1].route_id=="field_hub");
    const auto rename=graph.previewMarkerChange("hub","entrance","gate","west_gate"); REQUIRE(rename.rename_allowed); REQUIRE(graph.renameMarker(rename)); REQUIRE(graph.validate().empty());
    REQUIRE(graph.routes()[0].target_entrance_id=="west_gate"); REQUIRE(graph.routes()[1].target_entrance_id=="west_gate");
    const auto preview=graph.buildPreview(); REQUIRE(preview.nodes.size()==3); REQUIRE(preview.edges.size()==2); REQUIRE(preview.nodes[0].x==0); REQUIRE(preview.nodes[1].x==240);
}

TEST_CASE("World marker deletion requires a current reviewed impact and removes affected routes",
          "[map][world_graph][impact][delete]") {
    using namespace urpg::map;
    ProjectWorldGraph graph;
    REQUIRE(graph.addMap({"hub", "Hub", {{"gate", "Gate", 1, 1}}, {}, {}, {}}));
    REQUIRE(graph.addMap({"field", "Field", {}, {{"to_hub", "Hub", 2, 1}}, {}, {}}));
    REQUIRE(graph.addRoute({"field_hub", "Field to Hub", "field", "to_hub", "hub", "gate", ""}));
    auto stale = graph.previewMarkerChange("hub", "entrance", "gate");
    stale.affected_routes.clear();
    REQUIRE_FALSE(graph.deleteMarker(stale));
    REQUIRE(graph.maps()[1].entrances.size() == 1);
    const auto reviewed = graph.previewMarkerChange("hub", "entrance", "gate");
    REQUIRE(reviewed.affected_routes.size() == 1);
    REQUIRE(graph.deleteMarker(reviewed));
    REQUIRE(graph.routes().empty());
    REQUIRE(graph.maps()[1].entrances.empty());
    const auto restored = ProjectWorldGraph::fromJson(graph.toJson());
    REQUIRE(restored.has_value());
    REQUIRE(restored->toJson() == graph.toJson());
}
