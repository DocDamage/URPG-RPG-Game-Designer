#include "editor/spatial/grid_part_inspector_panel.h"
#include "editor/spatial/grid_part_palette_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/presentation/presentation_schema.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

using namespace urpg::editor;
using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  GridPartLayer layer = GridPartLayer::Object) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Editor test part";
    definition.category = category;
    definition.default_layer = layer;
    definition.footprint.width = 1;
    definition.footprint.height = 1;
    definition.asset_id = definition.part_id + ".asset";
    definition.default_properties["source"] = "catalog";
    return definition;
}

urpg::presentation::SpatialMapOverlay makeOverlay() {
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "map001";
    overlay.elevation.width = 8;
    overlay.elevation.height = 6;
    overlay.elevation.levels.assign(48, 0);
    return overlay;
}

PropPlacementPanel::ScreenProjectionSettings makeProjection() {
    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 800.0f;
    projection.viewportHeight = 600.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 3.0f;
    projection.worldUnitsPerPixel = 0.01f;
    return projection;
}

} // namespace

TEST_CASE("Grid part palette exposes deterministic selectable catalog entries", "[grid_part][editor]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    REQUIRE(catalog.addDefinition(makeDefinition("enemy.slime", GridPartCategory::Enemy, GridPartLayer::Actor)));

    GridPartPalettePanel palette;
    palette.SetCatalog(&catalog);
    palette.SetSearchQuery("slime");

    const auto& snapshot = palette.lastRenderSnapshot();
    REQUIRE(snapshot.has_catalog);
    REQUIRE(snapshot.part_count == 1);
    REQUIRE(snapshot.entries[0].part_id == "enemy.slime");

    REQUIRE(palette.SelectPart("enemy.slime"));
    REQUIRE(palette.lastRenderSnapshot().selected_part_id == "enemy.slime");
    REQUIRE(palette.lastRenderSnapshot().entries[0].selected);
}

TEST_CASE("Grid part placement previews places and undoes selected parts", "[grid_part][editor]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    GridPartPlacementPanel placement;
    placement.SetTargets(&document, &catalog, &overlay);
    placement.SetSelectedPartId("prop.crate");
    placement.SetProjectionSettings(makeProjection());

    REQUIRE(placement.HoverSelectedPartFromScreen(500.0f, 400.0f));
    auto snapshot = placement.lastRenderSnapshot();
    REQUIRE(snapshot.has_document);
    REQUIRE(snapshot.has_catalog);
    REQUIRE(snapshot.has_spatial_overlay);
    REQUIRE(snapshot.hover_active);
    REQUIRE(snapshot.hover_valid);
    REQUIRE(snapshot.hover_x == 5);
    REQUIRE(snapshot.hover_y == 4);

    REQUIRE(placement.PlaceSelectedPartFromScreen(500.0f, 400.0f));
    REQUIRE(document.parts().size() == 1);
    REQUIRE(document.parts()[0].instance_id == "map001:prop.crate:5:4");
    REQUIRE(document.parts()[0].properties.at("source") == "catalog");
    REQUIRE(placement.lastRenderSnapshot().placed_count == 1);
    REQUIRE(placement.lastRenderSnapshot().can_undo);

    REQUIRE(placement.Undo());
    REQUIRE(document.parts().empty());
    REQUIRE(placement.lastRenderSnapshot().can_redo);

    REQUIRE(placement.Redo());
    REQUIRE(document.findPart("map001:prop.crate:5:4") != nullptr);
}

TEST_CASE("Grid part inspector edits selected part properties through command history", "[grid_part][editor]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);

    PlacedPartInstance part;
    part.instance_id = "map001:prop.crate:1:2";
    part.part_id = "prop.crate";
    part.grid_x = 1;
    part.grid_y = 2;
    REQUIRE(document.placePart(part));

    GridPartInspectorPanel inspector;
    inspector.SetTargets(&document, &catalog);
    REQUIRE(inspector.SelectInstance("map001:prop.crate:1:2"));
    REQUIRE(inspector.SetProperty("lootTable", "crate_loot"));

    auto snapshot = inspector.lastRenderSnapshot();
    REQUIRE(snapshot.has_selection);
    REQUIRE(snapshot.selected_instance_id == "map001:prop.crate:1:2");
    REQUIRE(snapshot.properties.at("lootTable") == "crate_loot");
    REQUIRE(snapshot.can_undo);

    REQUIRE(inspector.Undo());
    REQUIRE_FALSE(document.findPart("map001:prop.crate:1:2")->properties.contains("lootTable"));
}

TEST_CASE("Spatial workspace routes Parts mode placement to the grid part editor", "[grid_part][editor][spatial]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(nullptr, &overlay);
    workspace.SetGridPartTargets(&document, &catalog);
    workspace.SetProjectionSettings(makeProjection());

    REQUIRE(workspace.ActivateToolbarAction("parts"));
    REQUIRE(workspace.SelectGridPart("prop.crate"));
    REQUIRE(workspace.RouteCanvasHover(500.0f, 400.0f));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));

    const auto& snapshot = workspace.lastRenderSnapshot();
    REQUIRE(workspace.activeMode() == SpatialAuthoringWorkspace::ToolMode::Parts);
    REQUIRE(snapshot.toolbar.active_mode == "parts");
    REQUIRE(snapshot.parts_placement.placed_count == 1);
    REQUIRE(document.findPart("map001:prop.crate:5:4") != nullptr);
}
