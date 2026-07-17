#include "editor/spatial/map_batch_edit.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/scene/map_scene.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Map batch editing plans every selection clipboard and shape operation", "[spatial][map_batch_edit]") {
    using namespace urpg::editor;
    const std::vector<MapTileCell> cells = {{"ground","town","grass",1,1},{"ground","town","grass",2,1},{"ground","town","grass",2,2}};
    REQUIRE(MapBatchEdit::select(cells, MapSelectionShape::Point, {{1,1}}).size() == 1);
    REQUIRE(MapBatchEdit::select(cells, MapSelectionShape::Box, {{1,1},{2,2}}).size() == 3);
    REQUIRE(MapBatchEdit::select(cells, MapSelectionShape::Lasso, {{0,0},{3,0},{3,3},{0,3}}).size() == 3);
    REQUIRE(MapBatchEdit::cut(cells).edits.size() == 3);
    REQUIRE(MapBatchEdit::paste(cells, 5, 5).edits[0].tile_x == 5);
    REQUIRE(MapBatchEdit::duplicate(cells, 2, 3).edits[0].tile_y == 4);
    REQUIRE(MapBatchEdit::move(cells, 2, 0).edits.size() == 6);
    REQUIRE(MapBatchEdit::fill("ground","town","water",{0,0},{2,2}).edits.size() == 9);
    REQUIRE(MapBatchEdit::line("ground","town","water",{0,0},{3,3}).edits.size() == 4);
    REQUIRE(MapBatchEdit::rectangle("ground","town","water",{0,0},{2,2}).edits.size() == 8);
    REQUIRE(MapBatchEdit::replace(cells,"town","water").destructive);
    REQUIRE(MapBatchEdit::stamp(cells,8,8).edits[0].tile_x == 8);
}

TEST_CASE("Map batch tile edits apply atomically with undo and save reopen", "[spatial][map_batch_edit][roundtrip]") {
    using namespace urpg::editor;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "batch_map"; overlay.elevation.width = 6; overlay.elevation.height = 6;
    overlay.elevation.levels.assign(36, 0);
    urpg::scene::MapScene map("batch_map", 6, 6);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);
    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    workspace.SetPerspectiveTilePaletteOptions(
        {{"grass", "Grass", "town", "grass", "", "", "terrain", ""}});
    const auto filled = MapBatchEdit::fill("ground", "town", "grass", {0,0}, {1,1});
    const auto applied = workspace.applyNativeTileEdits(workspace.perspectiveDocumentRevision(), filled.edits);
    REQUIRE(applied.success);
    REQUIRE(applied.applied_tile_count == 4);
    const std::vector<MapTileCell> left = {{"ground","town","grass",0,0},{"ground","town","grass",0,1}};
    const auto moved = MapBatchEdit::move(left, 2, 0);
    REQUIRE(moved.destructive);
    const auto moveApplied = workspace.applyNativeTileEdits(workspace.perspectiveDocumentRevision(), moved.edits);
    REQUIRE(moveApplied.success);
    REQUIRE(moveApplied.applied_tile_count == 4);
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);
    REQUIRE(workspace.UndoPerspective2D());
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);
    REQUIRE(workspace.RedoPerspective2D());
    const auto saved = workspace.PreparePerspectiveMapDraftSave();
    REQUIRE(saved.success);
    SpatialAuthoringWorkspace restored;
    REQUIRE(restored.LoadPerspectiveMapDraft(saved.serialized_document_json).success);
    REQUIRE(restored.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);
}

TEST_CASE("Map prop and event multi-selection applies batch properties atomically",
          "[spatial][map_batch_edit][objects][roundtrip]") {
    using namespace urpg::editor;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "object_batch"; overlay.elevation.width = 8; overlay.elevation.height = 8;
    overlay.elevation.levels.assign(64, 0);
    overlay.props.emplace_back("prop.a", "crate", 1.5f, 0.0f, 1.5f, 0.0f, 1.0f);
    overlay.props.emplace_back("prop.b", "barrel", 2.5f, 0.0f, 2.5f, 0.0f, 1.0f);
    urpg::scene::MapScene map("object_batch", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);
    workspace.SetProjectionSettings({800.0f, 600.0f, 4.0f, 4.0f, 1.0f / 48.0f, true});
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("event.a", "Vendor", "confirm_interact", 400.0f, 300.0f));
    const auto& event = workspace.lastRenderSnapshot().perspective_2d_events.front();
    const std::vector<MapPropCell> props = {{"prop.a",1,1},{"prop.b",2,2}};
    const std::vector<MapEventCell> events = {{event.event_id,event.tile_x,event.tile_y}};
    const auto selectedProps = MapBatchEdit::selectProps(props, MapSelectionShape::Box, {{0,0},{2,2}});
    const auto selectedEvents = MapBatchEdit::selectEvents(events, MapSelectionShape::Box, {{0,0},{7,7}});
    REQUIRE(selectedProps.size() == 2);
    REQUIRE(selectedEvents.size() == 1);
    const auto preview = MapBatchEdit::objectProperties(selectedProps, selectedEvents, 1, 1, 90.0f, 1.5f, true, false);
    REQUIRE(preview.valid);
    const auto applied = workspace.applyNativeObjectPropertyEdits(
        workspace.perspectiveDocumentRevision(), preview.prop_edits, preview.event_edits);
    REQUIRE(applied.success);
    REQUIRE(applied.applied_prop_count == 2);
    REQUIRE(applied.applied_event_count == 1);
    REQUIRE(overlay.props[0].posX == 2.5f);
    REQUIRE(overlay.props[0].rotY == 90.0f);
    REQUIRE(overlay.props[0].scale == 1.5f);
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_events.front().blocks_movement);
    REQUIRE_FALSE(workspace.lastRenderSnapshot().perspective_2d_events.front().sprite_visible);
    REQUIRE(workspace.UndoPerspective2D());
    REQUIRE(overlay.props[0].posX == 1.5f);
    REQUIRE(workspace.RedoPerspective2D());
    const auto saved = workspace.PreparePerspectiveMapDraftSave();
    REQUIRE(saved.success);
    SpatialAuthoringWorkspace reopened;
    REQUIRE(reopened.LoadPerspectiveMapDraft(saved.serialized_document_json).success);
    REQUIRE(reopened.lastRenderSnapshot().perspective_2d_events.front().blocks_movement);
}
