#include <catch2/catch_test_macros.hpp>
#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "engine/core/presentation/map_scene_translator.h"
#include "engine/core/presentation/presentation_migrate.h"
#include "engine/core/presentation/render_backend_mock.h"
#include "engine/core/presentation/presentation_schema.h"

using namespace urpg::editor;
using namespace urpg::presentation;

TEST_CASE("Spatial Editor Tooling Integration", "[editor][spatial]") {
    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);

    SECTION("ElevationBrush modifies grid data") {
        ElevationBrushPanel brush;
        brush.SetTarget(&overlay);
        
        brush.ApplyBrush(5, 5, 2);
        REQUIRE(overlay.elevation.levels[5 * 10 + 5] == 2);
        
        // Out of bounds safety
        brush.ApplyBrush(20, 20, 5); 
        // Should not crash
    }

    SECTION("PropPlacement adds instances") {
        PropPlacementPanel placement;
        placement.SetTarget(&overlay);
        
        placement.AddProp("rock_01", 1.5f, 0.0f, 2.5f);
        REQUIRE(overlay.props.size() == 1);
        REQUIRE(overlay.props[0].assetId == "rock_01");
        REQUIRE(overlay.props[0].posX == 1.5f);
    }

    SECTION("SpatialMapOverlay Serialization Round-trip") {
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

    SECTION("MapSceneTranslator emits Light and Fog intents") {
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
            if (cmd.type == PresentationCommand::Type::SetLight && cmd.id == 101) foundLight = true;
            if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile && cmd.fogProfile->density == 0.5f) foundFog = true;
            if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile && cmd.postFXProfile->bloomIntensity == 0.9f) foundPostFx = true;
            if (cmd.type == PresentationCommand::Type::DrawShadowProxy && cmd.id == 7) foundShadowProxy = true;
        }

        REQUIRE(foundLight);
        REQUIRE(foundFog);
        REQUIRE(foundPostFx);
        REQUIRE(foundShadowProxy);
    }

    SECTION("RenderBackendMock consumes environment commands") {
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
}
