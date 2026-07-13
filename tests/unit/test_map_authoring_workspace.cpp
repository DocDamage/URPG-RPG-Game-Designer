#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/map_authoring_workspace.h"
#include "editor/spatial/spatial_authoring_workspace.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MapAuthoringWorkspace routes release entry modes through existing child workspaces", "[spatial][map_authoring]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);
    workspace.setProjectRoot("C:/projects/creator-demo");
    workspace.setActiveMapId("map_intro");

    REQUIRE(workspace.activateMode(urpg::editor::MapAuthoringMode::Parts));
    REQUIRE(levelBuilder.activeMode() == urpg::editor::LevelBuilderWorkspace::WorkflowMode::Build);
    REQUIRE(workspace.snapshot().activeMode == "parts");

    REQUIRE(workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas));
    REQUIRE(levelBuilder.activeMode() == urpg::editor::LevelBuilderWorkspace::WorkflowMode::Perspective2D);
    REQUIRE(workspace.snapshot().hasLevelBuilder);
    REQUIRE(workspace.snapshot().hasPerspective2D);
    REQUIRE(workspace.snapshot().modes.size() == 10);

    workspace.context().recordCommand({"paint", urpg::editor::MapAuthoringDocumentOwner::Perspective2D, false, true});
    workspace.refresh();
    REQUIRE(workspace.snapshot().nextAction == "Save changed map documents before playtest.");

    workspace.setNextActionHint("Paint the starter map, then playtest it.");
    REQUIRE(workspace.snapshot().nextAction == "Paint the starter map, then playtest it.");
    workspace.clearNextActionHint();
    REQUIRE(workspace.snapshot().nextAction == "Save changed map documents before playtest.");

    workspace.setLayout({0.99f, 0.01f, 0.99f, false, true, false});
    REQUIRE(workspace.snapshot().layout.paletteWidthFraction == 0.35f);
    REQUIRE(workspace.snapshot().layout.inspectorWidthFraction == 0.12f);
    REQUIRE(workspace.snapshot().layout.diagnosticsHeightFraction == 0.40f);
    REQUIRE_FALSE(workspace.snapshot().layout.paletteVisible);

    REQUIRE(workspace.activateMode(urpg::editor::MapAuthoringMode::Package));
    REQUIRE(workspace.snapshot().nextAction == "Select a map part and set the player spawn before packaging.");
}

TEST_CASE("MapAuthoringWorkspace retains contextual selection until the creator returns", "[spatial][map_authoring][context_action]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);
    workspace.setProjectRoot("C:/projects/creator-demo");
    workspace.setActiveMapId("map_intro");
    auto selection = workspace.context().snapshot().selection;
    selection.eventId = "elder_mira";
    selection.viewportFocus = "tile:4,6";
    workspace.context().setSelection(selection);

    const auto opened = workspace.openContextAction({"event_authoring", "event", "elder_mira", {}, {}, "map"});
    REQUIRE(opened.success);
    REQUIRE(workspace.snapshot().activeContextRoute == "event_authoring");
    REQUIRE(workspace.snapshot().activeContextObjectId == "elder_mira");
    REQUIRE(workspace.snapshot().activeContextReturnRoute == "map");
    REQUIRE(workspace.activeContextAction() != nullptr);

    const auto returned = workspace.returnFromContextAction();
    REQUIRE(returned.success);
    REQUIRE(workspace.snapshot().activeContextRoute.empty());
    REQUIRE(workspace.context().snapshot().selection.viewportFocus == "tile:4,6");
}

TEST_CASE("MapAuthoringWorkspace accepts attached asset drops into durable Map palettes", "[spatial][map_authoring][assets]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);

    urpg::editor::EditorAssetDragPayload attached;
    attached.assetId = "asset.hero";
    attached.projectPath = "content/assets/imported/asset.hero/hero.png";
    attached.mediaKind = "image";
    attached.width = 48;
    attached.height = 48;
    attached.provenance = urpg::editor::EditorAssetProvenanceState::Attached;

    const auto tileDrop = workspace.acceptAssetDrop(attached, "tiles");
    REQUIRE(tileDrop.accepted);
    REQUIRE(workspace.context().snapshot().perspective2DDirty);
    REQUIRE_FALSE(workspace.context().snapshot().canUndo);
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);

    const auto propDrop = workspace.acceptAssetDrop(attached, "props");
    REQUIRE(propDrop.accepted);
    REQUIRE(perspective2D.lastRenderSnapshot().props.project_asset_options.size() == 1);

    attached.provenance = urpg::editor::EditorAssetProvenanceState::RawExternal;
    const auto rawDrop = workspace.acceptAssetDrop(attached, "tiles");
    REQUIRE_FALSE(rawDrop.accepted);
    REQUIRE(rawDrop.code == "asset_drop_requires_attachment");
}

TEST_CASE("MapAuthoringWorkspace routes Perspective 2D history through the active Map mode", "[spatial][map_authoring][history]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    perspective2D.SetTargets(nullptr, nullptr);
    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);
    REQUIRE(workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas));

    REQUIRE(perspective2D.AddPerspectiveLayer("ground", "Ground", "tile"));
    workspace.refresh();
    REQUIRE(workspace.snapshot().context.canUndo);
    REQUIRE(workspace.snapshot().context.historyOwner == "perspective_2d");
    const auto undone = workspace.undo();
    REQUIRE(undone.success);
    REQUIRE(undone.owner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_layers.empty());

    const auto redone = workspace.redo();
    REQUIRE(redone.success);
    REQUIRE(redone.owner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_layers.size() == 1);
}
