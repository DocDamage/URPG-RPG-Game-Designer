#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/scene/map_scene.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Perspective tile quality links seams collisions materials and unreachable entrances to cells",
          "[spatial][map_tile_quality]") {
    using Workspace = urpg::editor::SpatialAuthoringWorkspace;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId="quality"; overlay.elevation.width=4; overlay.elevation.height=3; overlay.elevation.levels.assign(12,0);
    urpg::scene::MapScene map("quality",4,3); Workspace workspace; workspace.SetTargets(&map,&overlay);
    REQUIRE(workspace.AddPerspectiveLayer("ground","Ground","tile"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"water_a","Water A","town","water_a","","","terrain","water_a.png"},
        {"water_b","Water B","town","water_b","","","terrain","water_b.png"},
        {"wall","Wall","town","wall","","","terrain","wall.png"}});
    Workspace::Perspective2DTileDefinition waterA; waterA.tileset_id="town"; waterA.tile_id="water_a"; waterA.page_id="town";
    waterA.autotile=true; waterA.autotile_kind="water"; waterA.preview_path="water_a.png";
    Workspace::Perspective2DTileDefinition waterB=waterA; waterB.tile_id="water_b"; waterB.preview_path="water_b.png"; waterB.passable_left=false;
    Workspace::Perspective2DTileDefinition wall=waterA; wall.tile_id="wall"; wall.autotile=false; wall.autotile_kind=""; wall.preview_path=""; wall.collision=true;
    REQUIRE(workspace.SetPerspectiveTileDefinition(waterA)); REQUIRE(workspace.SetPerspectiveTileDefinition(waterB));
    REQUIRE(workspace.SetPerspectiveTileDefinition(wall));
    const std::vector<Workspace::Perspective2DNativeTileEdit> edits = {
        {"ground","town","water_a",0,0},{"ground","town","water_b",1,0},
        {"ground","town","wall",2,0},{"ground","town","wall",2,1},{"ground","town","wall",2,2}};
    REQUIRE(workspace.applyNativeTileEdits(workspace.perspectiveDocumentRevision(),edits).success);
    const auto report=workspace.inspectPerspectiveTileQuality(0,1,{{3,1}});
    REQUIRE(report.navigation_complete); REQUIRE(report.painted_tile_count==5); REQUIRE(report.reachable_tile_count==6);
    const auto find=[&](const std::string& code){ return std::find_if(report.diagnostics.begin(),report.diagnostics.end(),
        [&](const auto& diagnostic){ return diagnostic.code==code; }); };
    const auto seam=find("autotile_seam_mismatch"); REQUIRE(seam!=report.diagnostics.end()); REQUIRE(seam->tile_x==0); REQUIRE(seam->layer_id=="ground");
    const auto collision=find("collision_passability_conflict"); REQUIRE(collision!=report.diagnostics.end()); REQUIRE(collision->tile_x==2); REQUIRE(collision->tile_id=="wall");
    REQUIRE(find("tile_material_missing")!=report.diagnostics.end());
    const auto unreachable=find("entrance_unreachable"); REQUIRE(unreachable!=report.diagnostics.end()); REQUIRE(unreachable->tile_x==3); REQUIRE(unreachable->tile_y==1);
}
