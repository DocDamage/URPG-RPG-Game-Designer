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

TEST_CASE("Spatial authoring output flows into presentation runtime frame generation", "[presentation][spatial][e2e]") {
    // 1. Set up a spatial map overlay with a flat 8x8 grid
    SpatialMapOverlay overlay;
    overlay.mapId = "e2e_village";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.stepHeight = 0.5f;
    overlay.elevation.levels.assign(64, 0);
    overlay.fog.density = 0.1f;
    overlay.postFX.exposure = 1.0f;

    // 2. Use ElevationBrushPanel to raise a hill at (4,4)
    ElevationBrushPanel brush;
    brush.SetTarget(&overlay);
    brush.SetBrushSize(1);
    brush.ApplyBrush(4, 4, 4); // level 4 => 2.0 world units

    // Verify the hill was applied
    REQUIRE(overlay.elevation.levels[4 * 8 + 4] == 4);
    REQUIRE(overlay.elevation.GetWorldHeight(4, 4) == 2.0f);

    // 3. Use PropPlacementPanel to place a prop on the hill
    PropPlacementPanel placement;
    placement.SetTarget(&overlay);
    const float hillHeight = overlay.elevation.GetWorldHeight(4, 4);
    placement.AddProp("house_01", 4.5f, hillHeight, 4.5f);

    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props[0].assetId == "house_01");
    // Prop Y should reflect the edited elevation
    REQUIRE(overlay.props[0].posY == Catch::Approx(2.0f));

    // 4. Feed into PresentationAuthoringData
    PresentationAuthoringData data;
    data.mapOverlays.push_back(overlay);
    data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    // 5. Set up PresentationContext with an actor standing on the hill
    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    context.mapState.mapId = "e2e_village";
    context.mapState.actors.push_back({1, "hero", 4.0f, 4.0f, false});

    // 6. Build the presentation frame
    PresentationRuntime runtime;
    const PresentationFrameIntent intent = runtime.BuildPresentationFrame(context, data);

    // 7. Verify the frame contains commands with elevation-resolved Y positions
    bool foundActor = false;
    bool foundProp = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 1) {
            foundActor = true;
            // Actor standing on tile (4,4) with elevation level 4 => 2.0 world units + 0.25 anchor offset
            CHECK(cmd.position.y == Catch::Approx(2.25f));
        }
        if (cmd.type == PresentationCommand::Type::DrawProp && cmd.id == 1) {
            // Prop id is index+1 in the translator; first prop has id==1
            foundProp = true;
            CHECK(cmd.position.y == Catch::Approx(2.0f));
        }
    }

    REQUIRE(foundActor);
    REQUIRE(foundProp);
    REQUIRE(intent.activePasses.size() == 3);
}

// ---------------------------------------------------------------------------
// S23-T04: Scene render-command coverage — Menu / UI / Status HUD / Chat
// ---------------------------------------------------------------------------

TEST_CASE("MenuSceneTranslator emits background shadow proxy command", "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.menuId = "main_menu";
    state.drawBackground = true;
    state.blurBackground = false;
    state.zDepth = 2.5f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundBackground = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy && cmd.id == 0xBB) {
            foundBackground = true;
            CHECK(cmd.position.z == Catch::Approx(2.5f));
        }
    }
    REQUIRE(foundBackground);
}

TEST_CASE("MenuSceneTranslator emits blur PostFX when blurBackground is set", "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.drawBackground = true;
    state.blurBackground = true;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundPostFX = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            foundPostFX = true;
            REQUIRE(cmd.postFXProfile != nullptr);
            CHECK(cmd.postFXProfile->bloomIntensity == Catch::Approx(5.0f));
        }
    }
    REQUIRE(foundPostFX);
}

TEST_CASE("MenuSceneTranslator emits no commands when drawBackground is false and blurBackground is false",
          "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.drawBackground = false;
    state.blurBackground = false;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    REQUIRE(intent.commands.empty());
}

TEST_CASE("StatusHUDTranslator emits no commands when invisible", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = false;
    state.activeActorId = 5;
    state.activeStatusIconCount = 3;
    state.showMinimap = true;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    REQUIRE(intent.commands.empty());
}

TEST_CASE("StatusHUDTranslator emits turn-indicator overlay for active actor", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 42;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundTurnIndicator = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.id == 42) {
            foundTurnIndicator = true;
            CHECK(cmd.effectColor[0] == Catch::Approx(1.0f));
            CHECK(cmd.effectColor[2] == Catch::Approx(0.5f)); // yellow tint
        }
    }
    REQUIRE(foundTurnIndicator);
}

TEST_CASE("StatusHUDTranslator emits one overlay command per status icon", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 0;
    state.activeStatusIconCount = 4;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    size_t iconCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            ++iconCount;
        }
    }
    REQUIRE(iconCount == 4);
}

TEST_CASE("StatusHUDTranslator emits minimap overlay when showMinimap is true", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 0;
    state.showMinimap = true;
    state.minimapOpacity = 0.75f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundMinimap = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.id == 0xFFFF) {
            foundMinimap = true;
            CHECK(cmd.effectIntensity == Catch::Approx(0.75f));
        }
    }
    REQUIRE(foundMinimap);
}

TEST_CASE("StatusHUDTranslator combined: turn indicator + status icons + minimap", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 7;
    state.activeStatusIconCount = 2;
    state.showMinimap = true;
    state.minimapOpacity = 1.0f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    // Expected: 1 turn indicator + 2 status icons + 1 minimap = 4 overlay commands
    size_t overlayCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect)
            ++overlayCount;
    }
    REQUIRE(overlayCount == 4);
}

TEST_CASE("Chat/Dialogue overlay scene produces shadow proxy for contrast background", "[presentation][scene][chat]") {
    // The DialogueTranslator is responsible for chat-scene render commands.
    // This test verifies that an active high-contrast dialogue state emits
    // a shadow-proxy background command (the chat contrast background).
    DialogueTranslator translator;
    DialogueState dialogueState;
    dialogueState.isActive = true;
    dialogueState.requireHighContrast = true;

    DialoguePresentationConfig config;
    config.sceneSaturationMultiplier = 0.4f;
    config.contrastBgAlpha = 0.85f;

    PresentationFrameIntent intent;
    // Apply readability modifiers (adds shadow proxy + PostFX saturation override)
    translator.ApplyReadability(dialogueState, config, intent);

    bool foundContrastProxy = false;
    bool foundSaturationOverride = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy) {
            foundContrastProxy = true;
        }
        if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile) {
            // Saturation should be reduced to sceneSaturationMultiplier
            foundSaturationOverride = cmd.postFXProfile->saturation < 1.0f;
        }
    }
    REQUIRE(foundContrastProxy);
    REQUIRE(foundSaturationOverride);
}

TEST_CASE("Chat/Dialogue overlay scene is silent when dialogue is inactive", "[presentation][scene][chat]") {
    DialogueTranslator translator;
    DialogueState dialogueState;
    dialogueState.isActive = false;

    DialoguePresentationConfig config;
    config.sceneSaturationMultiplier = 0.4f;
    config.contrastBgAlpha = 0.85f;

    PresentationFrameIntent intent;
    translator.ApplyReadability(dialogueState, config, intent);

    // Inactive dialogue must not inject any presentation commands
    REQUIRE(intent.commands.empty());
}
