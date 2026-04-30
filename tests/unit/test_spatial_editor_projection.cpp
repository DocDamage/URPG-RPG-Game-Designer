#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/spatial/spatial_ability_canvas_panel.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/presentation/dialogue_translator.h"
#include "engine/core/presentation/map_scene_translator.h"
#include "engine/core/presentation/menu_scene_translator.h"
#include "engine/core/presentation/presentation_migrate.h"
#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/render_backend_mock.h"
#include "engine/core/presentation/spatial_projection.h"
#include "engine/core/scene/map_scene.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>

using namespace urpg::editor;
using namespace urpg::presentation;
using urpg::Vector2f;

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement projects viewport center to camera-centered world position", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.elevation.levels[5 * 10 + 4] = 3;

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 1.0f;

    float worldX = -1.0f;
    float worldY = -1.0f;
    float worldZ = -1.0f;
    const bool projected =
        PropPlacementPanel::TryProjectScreenToGround(overlay, 100.0f, 50.0f, projection, worldX, worldY, worldZ);

    REQUIRE(projected);
    REQUIRE(worldX == 4.0f);
    REQUIRE(worldY == 1.5f);
    REQUIRE(worldZ == 5.0f);
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement converts screen offsets into world xz offsets", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;

    float worldX = -1.0f;
    float worldY = -1.0f;
    float worldZ = -1.0f;
    const bool projected =
        PropPlacementPanel::TryProjectScreenToGround(overlay, 120.0f, 40.0f, projection, worldX, worldY, worldZ);

    REQUIRE(projected);
    REQUIRE(worldX == 6.0f);
    REQUIRE(worldY == 0.0f);
    REQUIRE(worldZ == 4.0f);
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement adds projected props using sampled elevation", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.elevation.levels[6 * 10 + 7] = 2;

    PropPlacementPanel placement;
    placement.SetTarget(&overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;

    const bool placed = placement.AddPropFromScreen("oak_01", 130.0f, 60.0f, projection);

    REQUIRE(placed);
    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props[0].assetId == "oak_01");
    REQUIRE(overlay.props[0].posX == 7.0f);
    REQUIRE(overlay.props[0].posY == 1.0f);
    REQUIRE(overlay.props[0].posZ == 6.0f);
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement rejects screen projection without a target overlay", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    PropPlacementPanel placement;

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;

    REQUIRE_FALSE(placement.AddPropFromScreen("oak_01", 100.0f, 50.0f, projection));
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement rejects out-of-bounds projected props", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    PropPlacementPanel placement;
    placement.SetTarget(&overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.25f;
    projection.rejectOutOfBounds = true;

    REQUIRE_FALSE(placement.AddPropFromScreen("oak_01", -50.0f, 50.0f, projection));
    REQUIRE(overlay.props.empty());
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement adds projected props from camera ray intersection", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.assign(64, 0);
    overlay.elevation.stepHeight = 0.5f;
    overlay.elevation.levels[3 * 8 + 4] = 4;

    PropPlacementPanel placement;
    placement.SetTarget(&overlay);

    CameraState cameraState;
    cameraState.targetPos = {4.5f, 2.0f, 3.5f};
    cameraState.currentPitch = 45.0f;
    cameraState.currentYaw = 0.0f;
    cameraState.currentDistance = 8.0f;

    CameraProfile profile;
    profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
    profile.fov = 60.0f;

    const ViewportRect viewport{0.0f, 0.0f, 1600.0f, 900.0f};
    const Vector2f screenPoint{800.0f, 450.0f};

    const bool placed = placement.AddPropFromScreen("banner_01", screenPoint, viewport, cameraState, profile);

    REQUIRE(placed);
    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props[0].assetId == "banner_01");
    CHECK(overlay.props[0].posX == Catch::Approx(4.5f).margin(0.1f));
    CHECK(overlay.props[0].posY == Catch::Approx(2.0f).margin(0.05f));
    CHECK(overlay.props[0].posZ == Catch::Approx(3.5f).margin(0.1f));
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement rejects camera ray misses outside authored grid", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.assign(64, 0);

    PropPlacementPanel placement;
    placement.SetTarget(&overlay);

    CameraState cameraState;
    cameraState.targetPos = {20.0f, 0.0f, 20.0f};
    cameraState.currentPitch = 45.0f;
    cameraState.currentYaw = 0.0f;
    cameraState.currentDistance = 8.0f;

    CameraProfile profile;
    profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
    profile.fov = 60.0f;

    const ViewportRect viewport{0.0f, 0.0f, 1280.0f, 720.0f};
    const Vector2f screenPoint{640.0f, 360.0f};

    REQUIRE_FALSE(placement.AddPropFromScreen("banner_01", screenPoint, viewport, cameraState, profile));
    REQUIRE(overlay.props.empty());
}

TEST_CASE("Spatial Editor Tooling Integration - SpatialMapOverlay Serialization Round-trip", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.mapId = "test_map";
    overlay.fog.density = 0.12f;
    overlay.props.push_back({"pine_tree", 10, 0, 10, 0, 1.0f});

    auto json = urpg::presentation::PresentationMigrationTool::ToJson(overlay);
    auto revived = urpg::presentation::PresentationMigrationTool::FromJson(json);

    REQUIRE(revived.mapId == "test_map");
    REQUIRE(revived.elevation.width == 10);
    REQUIRE(revived.fog.density == 0.12f);
    REQUIRE(revived.props.size() == 1);
    REQUIRE(revived.props[0].assetId == "pine_tree");
}

TEST_CASE("Spatial Editor Tooling Integration - MapSceneTranslator emits Light and Fog intents", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    SpatialMapOverlay myMap;
    myMap.mapId = "test_env";
    myMap.fog.density = 0.5f;
    myMap.postFX.bloomIntensity = 0.9f;

    LightProfile light;
    light.lightId = 101;
    light.type = LightProfile::LightType::Point;
    light.position = {0, 5, 0};
    light.color[0] = 1.0f;
    light.color[1] = 1.0f;
    light.color[2] = 1.0f;
    light.intensity = 2.0f;
    light.range = 15.0f;
    myMap.lights.push_back(light);

    PresentationAuthoringData data;
    data.mapOverlays.push_back(myMap);
    data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    PresentationContext ctx;
    ctx.activeMode = PresentationMode::Spatial;
    ctx.activeTier = CapabilityTier::Tier1_Standard;

    MapSceneState state;
    state.mapId = "test_env";
    state.actors.push_back({7, "hero", 2.0f, 3.0f, false});

    PresentationFrameIntent intent;
    MapSceneTranslatorImpl translator;
    translator.Translate(ctx, data, state, intent);

    bool foundLight = false;
    bool foundFog = false;
    bool foundPostFx = false;
    bool foundShadowProxy = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetLight && cmd.id == 101)
            foundLight = true;
        if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile && cmd.fogProfile->density == 0.5f)
            foundFog = true;
        if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile &&
            cmd.postFXProfile->bloomIntensity == 0.9f)
            foundPostFx = true;
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy && cmd.id == 7)
            foundShadowProxy = true;
    }

    REQUIRE(foundLight);
    REQUIRE(foundFog);
    REQUIRE(foundPostFx);
    REQUIRE(foundShadowProxy);
}

TEST_CASE("Spatial Editor Tooling Integration - RenderBackendMock consumes environment commands", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    PresentationFrameIntent intent;

    FogProfile fog;
    fog.density = 0.2f;

    PostFXProfile fx;
    fx.bloomIntensity = 0.35f;

    intent.AddFog(fog);
    intent.AddPostFX(fx);
    intent.AddShadowProxy(88, {1.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f});

    urpg::render::RenderBackendMock backend;
    backend.ConsumeFrame(intent);

    REQUIRE(backend.GetCurrentState().fogEnabled);
    REQUIRE(backend.GetCurrentState().postFxEnabled);
    REQUIRE(backend.GetCurrentState().bloomIntensity == 0.35f);
    REQUIRE(backend.GetDrawCalls().size() == 1);
    REQUIRE(backend.GetDrawCalls()[0].type == "ShadowProxy");
}

TEST_CASE("Spatial Editor Tooling Integration - SpatialProjection resolves center-screen ray to the target point on flat ground", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    CameraState cameraState;
    cameraState.targetPos = {5.0f, 0.0f, 6.0f};
    cameraState.currentPitch = 45.0f;
    cameraState.currentYaw = 0.0f;
    cameraState.currentDistance = 10.0f;

    CameraProfile profile;
    profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
    profile.fov = 60.0f;

    const ViewportRect viewport{0.0f, 0.0f, 1280.0f, 720.0f};
    const urpg::Vector2f screenPoint{640.0f, 360.0f};

    const WorldRay ray = SpatialProjection::ScreenPointToWorldRay(screenPoint, viewport, cameraState, profile);
    const auto hit = SpatialProjection::IntersectGroundPlane(ray, 0.0f);

    REQUIRE(hit.has_value());
    CHECK(hit->x == Catch::Approx(5.0f).margin(0.05f));
    CHECK(hit->y == Catch::Approx(0.0f).margin(0.01f));
    CHECK(hit->z == Catch::Approx(6.0f).margin(0.05f));
}

TEST_CASE("Spatial Editor Tooling Integration - SpatialProjection intersects elevated tiles before the base plane", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.assign(64, 0);
    overlay.elevation.stepHeight = 0.5f;
    overlay.elevation.levels[3 * 8 + 4] = 4; // 2.0 world units at tile (4,3)

    CameraState cameraState;
    cameraState.targetPos = {4.5f, 2.0f, 3.5f};
    cameraState.currentPitch = 45.0f;
    cameraState.currentYaw = 0.0f;
    cameraState.currentDistance = 8.0f;

    CameraProfile profile;
    profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
    profile.fov = 60.0f;

    const ViewportRect viewport{0.0f, 0.0f, 1600.0f, 900.0f};
    const urpg::Vector2f screenPoint{800.0f, 450.0f};

    const WorldRay ray = SpatialProjection::ScreenPointToWorldRay(screenPoint, viewport, cameraState, profile);
    const auto hit = SpatialProjection::IntersectElevationGrid(ray, overlay.elevation);

    REQUIRE(hit.has_value());
    CHECK(hit->x == Catch::Approx(4.5f).margin(0.1f));
    CHECK(hit->y == Catch::Approx(2.0f).margin(0.05f));
    CHECK(hit->z == Catch::Approx(3.5f).margin(0.1f));
}
