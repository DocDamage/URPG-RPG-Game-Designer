#include "editor/spatial/grid_part_inspector_panel.h"
#include "editor/spatial/grid_part_palette_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/presentation/presentation_schema.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
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
    definition.supported_rulesets.push_back(GridPartRuleset::TopDownJRPG);
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

TEST_CASE("Level Builder is the native grid-part authoring workspace", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, &overlay);
    workspace.SetProjectionSettings(makeProjection());

    const auto& initial = workspace.lastRenderSnapshot();
    REQUIRE(initial.native_level_editor);
    REQUIRE(initial.grid_part_document_is_source_of_truth);
    REQUIRE(initial.legacy_spatial_tools_are_supporting);
    REQUIRE(initial.status == "ready");
    REQUIRE(initial.active_mode == "build");
    REQUIRE(initial.can_author);

    REQUIRE(workspace.SelectGridPart("prop.crate"));
    REQUIRE(workspace.RouteCanvasHover(500.0f, 400.0f));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));

    const auto& placed = workspace.lastRenderSnapshot();
    REQUIRE(workspace.activeMode() == LevelBuilderWorkspace::WorkflowMode::Build);
    REQUIRE(placed.placement.placed_count == 1);
    REQUIRE(placed.inspector.has_selection);
    REQUIRE(placed.inspector.selected_instance_id == "map001:prop.crate:5:4");
    REQUIRE(document.findPart("map001:prop.crate:5:4") != nullptr);
}

TEST_CASE("Level Builder embeds supporting spatial tools without making them primary",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, &overlay);
    workspace.SetProjectionSettings(makeProjection());

    REQUIRE(workspace.SelectGridPart("prop.crate"));
    REQUIRE(workspace.ActivateToolbarAction("supporting_spatial"));

    const auto& supporting = workspace.lastRenderSnapshot();
    REQUIRE(workspace.activeMode() == LevelBuilderWorkspace::WorkflowMode::SupportingSpatial);
    REQUIRE(supporting.active_mode == "supporting_spatial");
    REQUIRE(supporting.native_level_editor);
    REQUIRE(supporting.legacy_spatial_tools_are_supporting);
    REQUIRE(supporting.supporting_spatial.visible);
    REQUIRE(supporting.supporting_spatial.has_target_overlay);
    REQUIRE(supporting.supporting_spatial.parts_palette.selected_part_id == "prop.crate");
    REQUIRE(supporting.supporting_spatial.parts_placement.selected_part_id == "prop.crate");

    (void)workspace.RouteCanvasPrimaryAction(500.0f, 400.0f);
    REQUIRE(document.parts().empty());
}

TEST_CASE("Level Builder routes supporting spatial canvas tools through the native workspace",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, &overlay);
    workspace.SetProjectionSettings(makeProjection());

    workspace.supportingSpatialWorkspace().propPanel().SetSelectedAssetId("oak_01");
    REQUIRE(workspace.ActivateToolbarAction("supporting_props"));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));
    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props.back().assetId == "oak_01");
    REQUIRE(document.parts().empty());

    workspace.supportingSpatialWorkspace().elevationPanel().SetBrushHeight(3.0f);
    REQUIRE(workspace.ActivateToolbarAction("supporting_elevation"));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));
    REQUIRE(overlay.elevation.levels[4 * 8 + 5] == 3);

    const auto& snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.active_mode == "supporting_spatial");
    REQUIRE(snapshot.supporting_spatial.toolbar.active_mode == "elevation");
    const auto elevationAction = std::find_if(snapshot.actions.begin(), snapshot.actions.end(),
                                             [](const auto& action) { return action.id == "supporting_elevation"; });
    REQUIRE(elevationAction != snapshot.actions.end());
    REQUIRE(elevationAction->active);
}

TEST_CASE("Level Builder saves canonical grid-part drafts and clears dirty state",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, &overlay);
    workspace.SetProjectionSettings(makeProjection());

    REQUIRE(workspace.SelectGridPart("prop.crate"));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));
    REQUIRE_FALSE(document.dirtyChunks().empty());
    REQUIRE(workspace.lastRenderSnapshot().has_unsaved_changes);

    const auto saveResult = workspace.SaveLevelDraft();
    REQUIRE(saveResult.success);
    REQUIRE(saveResult.map_id == "map001");
    REQUIRE(saveResult.saved_part_count == 1);
    REQUIRE(saveResult.blocker_codes.empty());
    REQUIRE(saveResult.serialized_document_json.find("\"partId\": \"prop.crate\"") != std::string::npos);
    REQUIRE(document.dirtyChunks().empty());
    REQUIRE_FALSE(workspace.lastRenderSnapshot().has_unsaved_changes);
    REQUIRE(workspace.lastRenderSnapshot().last_save.success);
}

TEST_CASE("Level Builder save command reports document blockers without clearing dirty state",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);

    PlacedPartInstance missingPart;
    missingPart.instance_id = "map001:missing.part:1:1";
    missingPart.part_id = "missing.part";
    missingPart.grid_x = 1;
    missingPart.grid_y = 1;
    REQUIRE(document.placePart(missingPart));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto saveResult = workspace.SaveLevelDraft();
    REQUIRE_FALSE(saveResult.success);
    REQUIRE(saveResult.serialized_document_json.empty());
    REQUIRE_FALSE(saveResult.blocker_codes.empty());
    REQUIRE(std::find(saveResult.blocker_codes.begin(), saveResult.blocker_codes.end(), "part_definition_missing") !=
            saveResult.blocker_codes.end());
    REQUIRE_FALSE(document.dirtyChunks().empty());
    REQUIRE(workspace.lastRenderSnapshot().has_unsaved_changes);
}

TEST_CASE("Level Builder loads canonical grid-part drafts into the bound document",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument source("map001", 8, 6);
    auto sourceOverlay = makeOverlay();

    LevelBuilderWorkspace sourceWorkspace;
    sourceWorkspace.SetTargets(&source, &catalog, &sourceOverlay);
    sourceWorkspace.SetProjectionSettings(makeProjection());
    REQUIRE(sourceWorkspace.SelectGridPart("prop.crate"));
    REQUIRE(sourceWorkspace.RouteCanvasPrimaryAction(500.0f, 400.0f));
    const auto saveResult = sourceWorkspace.SaveLevelDraft();
    REQUIRE(saveResult.success);

    GridPartDocument target("map001", 8, 6);
    auto targetOverlay = makeOverlay();
    LevelBuilderWorkspace targetWorkspace;
    targetWorkspace.SetTargets(&target, &catalog, &targetOverlay);

    const auto loadResult = targetWorkspace.LoadLevelDraft(saveResult.serialized_document_json);
    REQUIRE(loadResult.success);
    REQUIRE(loadResult.map_id == "map001");
    REQUIRE(loadResult.loaded_part_count == 1);
    REQUIRE(target.findPart("map001:prop.crate:5:4") != nullptr);
    REQUIRE(target.dirtyChunks().empty());
    REQUIRE(targetWorkspace.activeMode() == LevelBuilderWorkspace::WorkflowMode::Build);
    REQUIRE(targetWorkspace.lastRenderSnapshot().last_load.success);
    REQUIRE_FALSE(targetWorkspace.lastRenderSnapshot().has_unsaved_changes);
}

TEST_CASE("Level Builder load command rejects unsafe drafts without replacing the document",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);

    PlacedPartInstance existing;
    existing.instance_id = "map001:prop.crate:1:1";
    existing.part_id = "prop.crate";
    existing.grid_x = 1;
    existing.grid_y = 1;
    REQUIRE(document.placePart(existing));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto mismatch = workspace.LoadLevelDraft(
        R"({"schemaVersion":1,"mapId":"other_map","width":8,"height":6,"chunkSize":16,"parts":[]})");
    REQUIRE_FALSE(mismatch.success);
    REQUIRE(std::find(mismatch.blocker_codes.begin(), mismatch.blocker_codes.end(), "draft_map_id_mismatch") !=
            mismatch.blocker_codes.end());
    REQUIRE(document.findPart("map001:prop.crate:1:1") != nullptr);

    const auto invalid = workspace.LoadLevelDraft("{not-json");
    REQUIRE_FALSE(invalid.success);
    REQUIRE(std::find(invalid.blocker_codes.begin(), invalid.blocker_codes.end(), "draft_json_parse_failed") !=
            invalid.blocker_codes.end());
    REQUIRE(document.findPart("map001:prop.crate:1:1") != nullptr);
}

TEST_CASE("Level Builder exposes top-level undo redo for placement edits", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    auto overlay = makeOverlay();

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, &overlay);
    workspace.SetProjectionSettings(makeProjection());

    REQUIRE(workspace.SelectGridPart("prop.crate"));
    REQUIRE(workspace.RouteCanvasPrimaryAction(500.0f, 400.0f));
    REQUIRE(document.findPart("map001:prop.crate:5:4") != nullptr);
    REQUIRE(workspace.lastRenderSnapshot().can_undo);

    const auto undo = workspace.UndoLastEdit();
    REQUIRE(undo.success);
    REQUIRE(undo.source == "placement");
    REQUIRE(document.parts().empty());
    REQUIRE(workspace.lastRenderSnapshot().can_redo);

    const auto redo = workspace.RedoLastEdit();
    REQUIRE(redo.success);
    REQUIRE(redo.source == "placement");
    REQUIRE(document.findPart("map001:prop.crate:5:4") != nullptr);
}

TEST_CASE("Level Builder exposes top-level undo redo for inspector edits", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);

    PlacedPartInstance part;
    part.instance_id = "map001:prop.crate:1:1";
    part.part_id = "prop.crate";
    part.grid_x = 1;
    part.grid_y = 1;
    REQUIRE(document.placePart(part));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);
    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:prop.crate:1:1"));
    REQUIRE(workspace.inspectorPanel().SetProperty("lootTable", "crate_loot"));

    const auto undo = workspace.UndoLastEdit();
    REQUIRE(undo.success);
    REQUIRE(undo.source == "inspector");
    REQUIRE_FALSE(document.findPart("map001:prop.crate:1:1")->properties.contains("lootTable"));

    const auto redo = workspace.RedoLastEdit();
    REQUIRE(redo.success);
    REQUIRE(redo.source == "inspector");
    REQUIRE(document.findPart("map001:prop.crate:1:1")->properties.at("lootTable") == "crate_loot");
}

TEST_CASE("Level Builder focuses diagnostics with instance targets", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    auto crate = makeDefinition("prop.crate", GridPartCategory::Prop);
    crate.supported_rulesets.clear();
    REQUIRE(catalog.addDefinition(crate));

    GridPartDocument document("map001", 8, 6);
    PlacedPartInstance part;
    part.instance_id = "map001:prop.crate:1:1";
    part.part_id = "prop.crate";
    part.grid_x = 1;
    part.grid_y = 1;
    REQUIRE(document.placePart(part));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto& snapshot = workspace.lastRenderSnapshot();
    const auto diagnostic = std::find_if(snapshot.diagnostics.begin(), snapshot.diagnostics.end(),
                                         [](const auto& row) {
                                             return row.code == "part_ruleset_incompatible" &&
                                                    row.instance_id == "map001:prop.crate:1:1";
                                         });
    REQUIRE(diagnostic != snapshot.diagnostics.end());

    const auto focus = workspace.FocusDiagnostic(
        static_cast<size_t>(std::distance(snapshot.diagnostics.begin(), diagnostic)));
    REQUIRE(focus.success);
    REQUIRE(focus.instance_id == "map001:prop.crate:1:1");
    REQUIRE(workspace.activeMode() == LevelBuilderWorkspace::WorkflowMode::Validate);
    REQUIRE(workspace.lastRenderSnapshot().inspector.has_selection);
    REQUIRE(workspace.lastRenderSnapshot().inspector.selected_instance_id == "map001:prop.crate:1:1");
}

TEST_CASE("Level Builder reports unfocusable global diagnostics", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto& snapshot = workspace.lastRenderSnapshot();
    const auto diagnostic = std::find_if(snapshot.diagnostics.begin(), snapshot.diagnostics.end(),
                                         [](const auto& row) {
                                             return row.code == "ruleset_requires_spawn" && row.instance_id.empty();
                                         });
    REQUIRE(diagnostic != snapshot.diagnostics.end());

    const auto focus = workspace.FocusDiagnostic(
        static_cast<size_t>(std::distance(snapshot.diagnostics.begin(), diagnostic)));
    REQUIRE_FALSE(focus.success);
    REQUIRE(focus.source == "ruleset");
    REQUIRE(focus.instance_id.empty());
    REQUIRE(workspace.lastRenderSnapshot().last_focus_diagnostic.message ==
            "Diagnostic has no instance target to focus.");
}

TEST_CASE("Level Builder provides native spawn and objective authoring commands",
          "[grid_part][editor][level_builder]") {
    auto spawn = makeDefinition("spawn.marker", GridPartCategory::Trigger, GridPartLayer::Object);
    auto exit = makeDefinition("door.exit", GridPartCategory::Door, GridPartLayer::Object);

    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(spawn));
    REQUIRE(catalog.addDefinition(exit));

    GridPartDocument document("map001", 8, 6);
    PlacedPartInstance spawnPart;
    spawnPart.instance_id = "map001:spawn.marker:0:0";
    spawnPart.part_id = "spawn.marker";
    spawnPart.category = GridPartCategory::Trigger;
    spawnPart.layer = GridPartLayer::Object;
    REQUIRE(document.placePart(spawnPart));

    PlacedPartInstance exitPart;
    exitPart.instance_id = "map001:door.exit:2:0";
    exitPart.part_id = "door.exit";
    exitPart.category = GridPartCategory::Door;
    exitPart.layer = GridPartLayer::Object;
    exitPart.grid_x = 2;
    REQUIRE(document.placePart(exitPart));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:spawn.marker:0:0"));
    const auto spawnResult = workspace.MarkSelectedInstanceAsPlayerSpawn();
    REQUIRE(spawnResult.success);
    REQUIRE(document.findPart("map001:spawn.marker:0:0")->properties.at("role") == "player_spawn");

    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:door.exit:2:0"));
    const auto objectiveResult = workspace.SetSelectedInstanceAsReachExitObjective("leave_map");
    REQUIRE(objectiveResult.success);
    REQUIRE(document.findPart("map001:door.exit:2:0")->properties.at("role") == "exit");

    const auto& snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.last_authoring_command.command_id == "set_reach_exit_objective");
    REQUIRE(snapshot.validation.ruleset_ok);
    REQUIRE(snapshot.validation.objective_ok);
    REQUIRE_FALSE(std::any_of(snapshot.diagnostics.begin(), snapshot.diagnostics.end(), [](const auto& row) {
        return row.code == "ruleset_requires_spawn" || row.code == "objective_missing";
    }));

    REQUIRE(workspace.ActivateToolbarAction("playtest_start"));
    const auto& playtested = workspace.lastRenderSnapshot();
    REQUIRE(playtested.playtest.latest_result.completed_objective);
    REQUIRE(playtested.readiness_evidence.has_player_spawn);
    REQUIRE(playtested.readiness_evidence.has_objective);
    REQUIRE(playtested.readiness_evidence.reachability_passed);
}

TEST_CASE("Level Builder spawn authoring command requires a selection", "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("spawn.marker", GridPartCategory::Trigger, GridPartLayer::Object)));
    GridPartDocument document("map001", 8, 6);

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto result = workspace.MarkSelectedInstanceAsPlayerSpawn();
    REQUIRE_FALSE(result.success);
    REQUIRE(result.message == "Select a placed grid part before marking a player spawn.");
    REQUIRE_FALSE(workspace.lastRenderSnapshot().last_authoring_command.success);
}

TEST_CASE("Level Builder promotes successful native playtests into package readiness evidence",
          "[grid_part][editor][level_builder]") {
    auto spawn = makeDefinition("spawn.marker", GridPartCategory::Trigger, GridPartLayer::Object);
    auto exit = makeDefinition("door.exit", GridPartCategory::Door, GridPartLayer::Object);

    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(spawn));
    REQUIRE(catalog.addDefinition(exit));

    GridPartDocument document("map001", 8, 6);
    PlacedPartInstance spawnPart;
    spawnPart.instance_id = "map001:spawn.marker:0:0";
    spawnPart.part_id = "spawn.marker";
    spawnPart.category = GridPartCategory::Trigger;
    spawnPart.layer = GridPartLayer::Object;
    REQUIRE(document.placePart(spawnPart));

    PlacedPartInstance exitPart;
    exitPart.instance_id = "map001:door.exit:2:0";
    exitPart.part_id = "door.exit";
    exitPart.category = GridPartCategory::Door;
    exitPart.layer = GridPartLayer::Object;
    exitPart.grid_x = 2;
    REQUIRE(document.placePart(exitPart));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);
    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:spawn.marker:0:0"));
    REQUIRE(workspace.MarkSelectedInstanceAsPlayerSpawn().success);
    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:door.exit:2:0"));
    REQUIRE(workspace.SetSelectedInstanceAsReachExitObjective().success);

    GridPartPackageManifest manifest;
    manifest.package_id = "map001";
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "spawn.marker.asset", true, true, true, false, false, "editor_test_asset"});
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "door.exit.asset", true, true, true, false, false, "editor_test_asset"});
    workspace.SetPackageManifest(manifest);

    REQUIRE(workspace.ActivateToolbarAction("playtest_start"));
    const auto& afterPlaytest = workspace.lastRenderSnapshot();
    REQUIRE(afterPlaytest.readiness_evidence.reachability_passed);

    REQUIRE(workspace.ActivateToolbarAction("package"));
    const auto& packaged = workspace.lastRenderSnapshot();
    REQUIRE(packaged.package.can_publish);
    REQUIRE_FALSE(packaged.package.can_export);
    REQUIRE(packaged.package.readiness == "publishable");
}

TEST_CASE("Level Builder readiness evidence commands unlock certified export", "[grid_part][editor][level_builder]") {
    auto spawn = makeDefinition("spawn.marker", GridPartCategory::Trigger, GridPartLayer::Object);
    auto exit = makeDefinition("door.exit", GridPartCategory::Door, GridPartLayer::Object);

    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(spawn));
    REQUIRE(catalog.addDefinition(exit));

    GridPartDocument document("map001", 8, 6);
    PlacedPartInstance spawnPart;
    spawnPart.instance_id = "map001:spawn.marker:0:0";
    spawnPart.part_id = "spawn.marker";
    spawnPart.category = GridPartCategory::Trigger;
    spawnPart.layer = GridPartLayer::Object;
    REQUIRE(document.placePart(spawnPart));

    PlacedPartInstance exitPart;
    exitPart.instance_id = "map001:door.exit:2:0";
    exitPart.part_id = "door.exit";
    exitPart.category = GridPartCategory::Door;
    exitPart.layer = GridPartLayer::Object;
    exitPart.grid_x = 2;
    REQUIRE(document.placePart(exitPart));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);
    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:spawn.marker:0:0"));
    REQUIRE(workspace.MarkSelectedInstanceAsPlayerSpawn().success);
    REQUIRE(workspace.inspectorPanel().SelectInstance("map001:door.exit:2:0"));
    REQUIRE(workspace.SetSelectedInstanceAsReachExitObjective().success);

    GridPartPackageManifest manifest;
    manifest.package_id = "map001";
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "spawn.marker.asset", true, true, true, false, false, "editor_test_asset"});
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "door.exit.asset", true, true, true, false, false, "editor_test_asset"});
    workspace.SetPackageManifest(manifest);

    REQUIRE(workspace.ActivateToolbarAction("playtest_start"));
    REQUIRE(workspace.MarkTargetExportChecksPassed().success);
    REQUIRE(workspace.MarkAccessibilityChecksPassed().success);
    REQUIRE(workspace.MarkPerformanceBudgetPassed().success);

    const auto& certified = workspace.lastRenderSnapshot();
    REQUIRE(certified.readiness_evidence.target_export_checks_passed);
    REQUIRE(certified.readiness_evidence.accessibility_checks_passed);
    REQUIRE(certified.readiness_evidence.performance_budget_passed);
    REQUIRE(certified.package.readiness == "certified");
    REQUIRE(certified.package.can_export);
    REQUIRE(certified.can_export_current_level);

    const auto exportResult = workspace.ExportCurrentLevel();
    REQUIRE(exportResult.success);
    REQUIRE(exportResult.readiness == "certified");
}

TEST_CASE("Level Builder exposes native validation playtest and package readiness",
          "[grid_part][editor][level_builder]") {
    auto spawn = makeDefinition("spawn.player", GridPartCategory::Trigger, GridPartLayer::Object);
    spawn.default_properties["role"] = "player_spawn";
    auto exit = makeDefinition("door.exit", GridPartCategory::Door, GridPartLayer::Object);

    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(spawn));
    REQUIRE(catalog.addDefinition(exit));

    GridPartDocument document("map001", 8, 6);
    PlacedPartInstance spawnPart;
    spawnPart.instance_id = "map001:spawn.player:0:0";
    spawnPart.part_id = "spawn.player";
    spawnPart.category = GridPartCategory::Trigger;
    spawnPart.layer = GridPartLayer::Object;
    spawnPart.properties["role"] = "player_spawn";
    REQUIRE(document.placePart(spawnPart));

    PlacedPartInstance exitPart;
    exitPart.instance_id = "map001:door.exit:2:0";
    exitPart.part_id = "door.exit";
    exitPart.category = GridPartCategory::Door;
    exitPart.layer = GridPartLayer::Object;
    exitPart.grid_x = 2;
    REQUIRE(document.placePart(exitPart));

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    MapObjective objective;
    objective.type = MapObjectiveType::ReachExit;
    objective.objective_id = "reach_exit";
    objective.target_instance_id = "map001:door.exit:2:0";
    workspace.SetObjective(objective);

    GridPartReadinessEvidence evidence;
    evidence.has_player_spawn = true;
    evidence.has_objective = true;
    evidence.reachability_passed = true;
    evidence.target_export_checks_passed = true;
    evidence.accessibility_checks_passed = true;
    evidence.performance_budget_passed = true;
    workspace.SetReadinessEvidence(evidence);

    GridPartPackageManifest manifest;
    manifest.package_id = "map001";
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "spawn.player.asset", true, true, true, false, false, "editor_test_asset"});
    manifest.dependencies.push_back(
        {GridPartDependencyType::Asset, "door.exit.asset", true, true, true, false, false, "editor_test_asset"});
    workspace.SetPackageManifest(manifest);

    REQUIRE(workspace.ActivateToolbarAction("validate"));
    const auto& validated = workspace.lastRenderSnapshot();
    REQUIRE(validated.active_mode == "validate");
    REQUIRE(validated.validation.document_ok);
    REQUIRE(validated.validation.objective_ok);
    REQUIRE(validated.validation.blocking_count == 0);

    REQUIRE(workspace.ActivateToolbarAction("playtest_start"));
    const auto& playtested = workspace.lastRenderSnapshot();
    REQUIRE(playtested.active_mode == "playtest");
    REQUIRE(playtested.playtest.running);
    REQUIRE(playtested.playtest.has_runtime);
    REQUIRE(playtested.playtest.latest_result.completed_objective);

    REQUIRE(workspace.ActivateToolbarAction("package"));
    const auto& packaged = workspace.lastRenderSnapshot();
    REQUIRE(packaged.active_mode == "package");
    REQUIRE(packaged.package.can_publish);
    REQUIRE(packaged.package.can_export);
    REQUIRE(packaged.can_export_current_level);
    REQUIRE(packaged.diagnostics.empty());

    const auto exportResult = workspace.ExportCurrentLevel();
    REQUIRE(exportResult.success);
    REQUIRE(exportResult.map_id == "map001");
    REQUIRE(exportResult.readiness == "certified");
    REQUIRE(exportResult.blocker_codes.empty());
    REQUIRE(exportResult.serialized_document_json.find("\"mapId\": \"map001\"") != std::string::npos);
    REQUIRE(workspace.lastRenderSnapshot().last_export.success);
}

TEST_CASE("Level Builder export command reports blockers without serializing unsafe levels",
          "[grid_part][editor][level_builder]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);

    LevelBuilderWorkspace workspace;
    workspace.SetTargets(&document, &catalog, nullptr);

    const auto exportResult = workspace.ExportCurrentLevel();
    REQUIRE_FALSE(exportResult.success);
    REQUIRE(exportResult.map_id == "map001");
    REQUIRE(exportResult.serialized_document_json.empty());
    REQUIRE_FALSE(exportResult.blocker_codes.empty());
    REQUIRE(std::find(exportResult.blocker_codes.begin(), exportResult.blocker_codes.end(), "ruleset_requires_spawn") !=
            exportResult.blocker_codes.end());
    REQUIRE(workspace.activeMode() == LevelBuilderWorkspace::WorkflowMode::Package);

    const auto& snapshot = workspace.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.diagnostics.empty());
    REQUIRE(std::any_of(snapshot.diagnostics.begin(), snapshot.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.source == "ruleset" && diagnostic.code == "ruleset_requires_spawn" && diagnostic.blocking &&
               diagnostic.severity == "error";
    }));
    REQUIRE(std::any_of(snapshot.diagnostics.begin(), snapshot.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.source == "objective" && diagnostic.code == "objective_missing" && diagnostic.blocking;
    }));
}
