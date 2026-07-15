#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/map_authoring_workspace.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/scene/map_scene.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

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

TEST_CASE("MapAuthoringWorkspace accepts attached asset drops into durable Map palettes", "[spatial][map_authoring][assets]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "creator_demo";
    overlay.elevation.width = 16;
    overlay.elevation.height = 12;
    overlay.elevation.levels.assign(16 * 12, 0);
    urpg::scene::MapScene mapScene("creator_demo", 16, 12);
    perspective2D.SetTargets(&mapScene, &overlay);
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
    REQUIRE(workspace.context().snapshot().canUndo);
    REQUIRE(workspace.context().snapshot().historyOwner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);

    const auto persisted = perspective2D.PreparePerspectiveMapDraftSave();
    REQUIRE(persisted.success);
    const auto persistedJson = nlohmann::json::parse(persisted.serialized_document_json);
    REQUIRE(persistedJson["tile_palette"].size() == 1);
    REQUIRE(persistedJson["tile_palette"][0]["asset_id"] == "asset.hero");

    const auto undone = workspace.undo();
    REQUIRE(undone.success);
    REQUIRE(undone.owner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.empty());

    const auto redone = workspace.redo();
    REQUIRE(redone.success);
    REQUIRE(redone.owner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);

    const auto propDrop = workspace.acceptAssetDrop(attached, "props");
    REQUIRE(propDrop.accepted);
    REQUIRE(perspective2D.lastRenderSnapshot().props.project_asset_options.size() == 1);

    const auto roundTrip = perspective2D.PreparePerspectiveMapDraftSave();
    REQUIRE(roundTrip.success);
    urpg::editor::SpatialAuthoringWorkspace restored;
    REQUIRE(restored.LoadPerspectiveMapDraft(roundTrip.serialized_document_json).success);
    REQUIRE(restored.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);
    REQUIRE(restored.lastRenderSnapshot().props.project_asset_options.size() == 1);

    attached.provenance = urpg::editor::EditorAssetProvenanceState::RawExternal;
    const auto rawDrop = workspace.acceptAssetDrop(attached, "tiles");
    REQUIRE_FALSE(rawDrop.accepted);
    REQUIRE(rawDrop.code == "asset_drop_requires_attachment");
}

TEST_CASE("MapAuthoringWorkspace atomically places an attached tile drop with owner-aware undo",
          "[spatial][map_authoring][assets][history]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "creator_demo";
    overlay.elevation.width = 16;
    overlay.elevation.height = 12;
    overlay.elevation.levels.assign(16 * 12, 0);
    urpg::scene::MapScene mapScene("creator_demo", 16, 12);
    perspective2D.SetTargets(&mapScene, &overlay);
    perspective2D.SetProjectionSettings({1280.0f, 720.0f, 8.0f, 6.0f, 1.0f / 48.0f, true});
    REQUIRE(perspective2D.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(perspective2D.SelectPerspectiveLayer("ground"));
    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);

    urpg::editor::EditorAssetDragPayload attached;
    attached.assetId = "asset.hero";
    attached.projectPath = "content/assets/imported/asset.hero/hero.png";
    attached.mediaKind = "image";
    attached.provenance = urpg::editor::EditorAssetProvenanceState::Attached;

    const auto placed = workspace.placeAssetDrop(attached, "tiles", 640.0f, 360.0f);
    REQUIRE(placed.accepted);
    REQUIRE(placed.code == "asset_drop_placed");
    REQUIRE(workspace.context().snapshot().perspective2DDirty);
    REQUIRE(workspace.context().snapshot().historyOwner == "perspective_2d");
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 1);

    const auto undone = workspace.undo();
    REQUIRE(undone.success);
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.empty());
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 0);

    const auto redone = workspace.redo();
    REQUIRE(redone.success);
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_palette.tile_options.size() == 1);
    REQUIRE(perspective2D.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 1);
}

TEST_CASE("MapAuthoringWorkspace projects attached event drops into the active MapScene",
          "[spatial][map_authoring][assets][events][render]") {
    urpg::editor::LevelBuilderWorkspace levelBuilder;
    urpg::editor::SpatialAuthoringWorkspace perspective2D;
    urpg::presentation::SpatialMapOverlay overlay;
    overlay.mapId = "creator_demo";
    overlay.elevation.width = 16;
    overlay.elevation.height = 12;
    overlay.elevation.levels.assign(16 * 12, 0);
    urpg::scene::MapScene mapScene("creator_demo", 16, 12);
    perspective2D.SetTargets(&mapScene, &overlay);
    perspective2D.SetProjectionSettings({1280.0f, 720.0f, 8.0f, 6.0f, 1.0f / 48.0f, true});
    REQUIRE(perspective2D.AddPerspectiveLayer("events", "Events", "event"));

    urpg::editor::MapAuthoringWorkspace workspace;
    workspace.bind(&levelBuilder, &perspective2D);
    REQUIRE(workspace.activateMode(urpg::editor::MapAuthoringMode::Events));

    urpg::editor::EditorAssetDragPayload attached;
    attached.assetId = "asset.vendor";
    attached.projectPath = "content/assets/imported/asset.vendor/vendor.png";
    attached.mediaKind = "image";
    attached.provenance = urpg::editor::EditorAssetProvenanceState::Attached;

    const auto placed = workspace.placeAssetDrop(attached, "events", 640.0f, 360.0f);
    REQUIRE(placed.accepted);
    REQUIRE(mapScene.eventSprites().size() == 1);
    const auto& event = perspective2D.lastRenderSnapshot().perspective_2d_events[0];
    REQUIRE(mapScene.eventSprites()[0].event_id == event.event_id);
    REQUIRE(mapScene.eventSprites()[0].asset.id == attached.assetId);
    REQUIRE(mapScene.eventSprites()[0].tile_x == event.tile_x);
    REQUIRE(mapScene.eventSprites()[0].tile_y == event.tile_y);
    REQUIRE(perspective2D.SetPerspectiveEventSpriteAnimation(event.event_id, 24, 32, 4, 0.20f, false));
    REQUIRE(mapScene.eventSprites()[0].frame_width == 24);
    REQUIRE(mapScene.eventSprites()[0].frame_height == 32);
    REQUIRE(mapScene.eventSprites()[0].frame_count == 4);
    REQUIRE(mapScene.eventSprites()[0].frame_duration == 0.20f);
    REQUIRE_FALSE(mapScene.eventSprites()[0].loop);

    REQUIRE(workspace.undo().success);
    REQUIRE(mapScene.eventSprites().size() == 1);
    REQUIRE(mapScene.eventSprites()[0].frame_width == 48);
    REQUIRE(mapScene.eventSprites()[0].frame_count == 1);
    REQUIRE(mapScene.eventSprites()[0].loop);
    REQUIRE(workspace.undo().success);
    REQUIRE(mapScene.eventSprites().empty());
    REQUIRE(workspace.redo().success);
    REQUIRE(mapScene.eventSprites().size() == 1);
    REQUIRE(mapScene.eventSprites()[0].frame_width == 48);
    REQUIRE(workspace.redo().success);
    REQUIRE(mapScene.eventSprites()[0].frame_width == 24);
    REQUIRE(mapScene.eventSprites()[0].frame_count == 4);
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
