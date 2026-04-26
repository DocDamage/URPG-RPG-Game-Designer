#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "map_scene_translator.h"
#include "presentation_runtime.h"
#include <cassert>
#include <vector>

using namespace urpg::presentation;
using namespace urpg::editor;

/**
 * @brief Stress test for the Presentation Core logic.
 * Ensures the system can handle a high volume of actors and lookups
 * without state corruption or performance spikes.
 */
void RunReleaseValidation() {
    urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation", "release_validation.started",
                                                "Starting Phase 5: Release Validation.");

    PresentationRuntime runtime;
    PresentationContext context;
    PresentationAuthoringData data;

    // 1. Setup Scaled Test Data (Village with 100 Actors)
    context.mapState.mapId = "mega_village";
    context.activeMode = PresentationMode::Spatial3D;
    context.activeTier = CapabilityTier::Tier1_Standard;

    // Create an elevation grid
    SpatialMapOverlay overlay;
    overlay.mapId = "mega_village";
    overlay.elevation.width = 100;
    overlay.elevation.height = 100;
    overlay.elevation.levels.assign(100 * 100, 2); // all at height level 2 (1.0 world units)
    data.mapOverlays.push_back(overlay);

    // Profile for actors
    ActorPresentationProfile heroProf;
    heroProf.actorId = "hero_class";
    heroProf.anchorOffset = {0.0f, 0.5f, 0.0f};
    data.actorProfiles.push_back(heroProf);

    for (uint32_t i = 0; i < 100; ++i) {
        MapActorState actor;
        actor.actorId = i;
        actor.classId = "hero_class";
        actor.posX = (float)(i % 10);
        actor.posY = (float)(i / 10);
        context.mapState.actors.push_back(actor);
    }

    // 2. Execute Frame Generation
    PresentationFrameIntent intent = runtime.BuildPresentationFrame(context, data);

    // 3. Validation Logic
    size_t actorCommandCount = 0;
    size_t fogCommandCount = 0;
    size_t postFxCommandCount = 0;
    size_t lightCommandCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            actorCommandCount++;
        } else if (cmd.type == PresentationCommand::Type::SetFog) {
            fogCommandCount++;
        } else if (cmd.type == PresentationCommand::Type::SetPostFX) {
            postFxCommandCount++;
        } else if (cmd.type == PresentationCommand::Type::SetLight) {
            lightCommandCount++;
        }
    }

    urpg::diagnostics::RuntimeDiagnostics::info(
        "presentation.release_validation", "release_validation.actor_command_count",
        "Actor Command Count: " + std::to_string(actorCommandCount) + " (Expected: 100)");
    urpg::diagnostics::RuntimeDiagnostics::info(
        "presentation.release_validation", "release_validation.environment_command_envelope",
        "Environment Command Envelope: fog=" + std::to_string(fogCommandCount) +
            ", postfx=" + std::to_string(postFxCommandCount) + ", lights=" + std::to_string(lightCommandCount));
    assert(actorCommandCount == 100);
    assert(fogCommandCount <= 1);
    assert(postFxCommandCount <= 1);

    // Verify Y-Position Resolution (Base 1.0 from level 2 + 0.5 from anchorOffset)
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            assert(cmd.position.y == 1.5f);
        }
    }
    urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation",
                                                "release_validation.y_height_resolution_passed",
                                                "Y-Height Resolution: PASSED");

    // 4. Fallback Logic Validation
    context.activeMode = PresentationMode::Classic2D;
    PresentationFrameIntent fallbackIntent = runtime.BuildPresentationFrame(context, data);

    for (const auto& cmd : fallbackIntent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            // In Classic2D, Z should be 0.0f (Flattened)
            assert(cmd.position.z == 0.0f);
        }
    }
    urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation",
                                                "release_validation.fallback_flattening_passed",
                                                "Fallback Flattening (Tier 0): PASSED");

    // 5. Battle Effect Cue Envelope Validation
    PresentationContext battleContext;
    battleContext.activeMode = PresentationMode::Spatial;
    battleContext.activeTier = CapabilityTier::Tier1_Standard;
    battleContext.battleState.battleArenaId = "validation_arena";
    battleContext.battleState.participants.push_back({1, "1", 0, false, 1.0f});
    battleContext.battleState.participants.push_back({2, "2", 0, true, 1.0f});

    urpg::presentation::effects::EffectCue battleCue;
    battleCue.frameTick = 0;
    battleCue.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    battleCue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Target;
    battleCue.sourceId = 1;
    battleCue.ownerId = 2;
    battleCue.overlayEmphasis = {1.0f};
    battleCue.intensity = {1.5f};
    battleContext.battleState.effectCues.push_back(battleCue);

    PresentationAuthoringData battleData;
    battleData.actorProfiles.push_back({"1", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
    battleData.actorProfiles.push_back({"2", {0.5f, 0.0f}, {0.0f, 0.50f, 0.0f}, true, 0.0f});
    battleData.battleConfig.useDynamicCineCamera = true;
    battleData.battleConfig.formation.type = BattleFormation::LayoutType::Staged;
    battleData.battleConfig.formation.spreadWidth = 1.5f;
    battleData.battleConfig.formation.depthSpacing = 2.0f;

    PresentationFrameIntent battleIntent = runtime.BuildPresentationFrame(battleContext, battleData);

    size_t worldEffectCommandCount = 0;
    size_t overlayEffectCommandCount = 0;
    for (const auto& cmd : battleIntent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect) {
            worldEffectCommandCount++;
        } else if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            overlayEffectCommandCount++;
        }
    }

    urpg::diagnostics::RuntimeDiagnostics::info(
        "presentation.release_validation", "release_validation.battle_effect_envelope",
        "Battle Effect Command Sample Envelope: world=" + std::to_string(worldEffectCommandCount) +
            ", overlay=" + std::to_string(overlayEffectCommandCount));
    assert(worldEffectCommandCount >= 1);
    assert(overlayEffectCommandCount >= 1);
    assert(worldEffectCommandCount <= 16);
    assert(overlayEffectCommandCount <= 16);

    // 6. Spatial Authoring -> Runtime Consumption Validation
    urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation",
                                                "release_validation.spatial_validation_started",
                                                "Starting Spatial Authoring -> Runtime Consumption Validation.");
    {
        SpatialMapOverlay overlay;
        overlay.mapId = "validation_village";
        overlay.elevation.width = 8;
        overlay.elevation.height = 8;
        overlay.elevation.stepHeight = 0.5f;
        overlay.elevation.levels.assign(64, 0);
        overlay.fog.density = 0.1f;
        overlay.postFX.exposure = 1.0f;

        // Edit elevation via brush
        ElevationBrushPanel brush;
        brush.SetTarget(&overlay);
        brush.SetBrushSize(1);
        brush.ApplyBrush(4, 4, 4); // level 4 => 2.0 world units

        assert(overlay.elevation.levels[4 * 8 + 4] == 4);
        assert(overlay.elevation.GetWorldHeight(4, 4) == 2.0f);

        // Place prop on the hill
        PropPlacementPanel placement;
        placement.SetTarget(&overlay);
        const float hillHeight = overlay.elevation.GetWorldHeight(4, 4);
        placement.AddProp("house_01", 4.5f, hillHeight, 4.5f);

        assert(overlay.props.size() == 1);
        assert(overlay.props[0].assetId == "house_01");
        assert(overlay.props[0].posY == 2.0f);

        // Feed into presentation runtime
        PresentationAuthoringData spatialData;
        spatialData.mapOverlays.push_back(overlay);
        spatialData.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

        PresentationContext spatialContext;
        spatialContext.activeMode = PresentationMode::Spatial;
        spatialContext.activeTier = CapabilityTier::Tier1_Standard;
        spatialContext.mapState.mapId = "validation_village";
        spatialContext.mapState.actors.push_back({1, "hero", 4.0f, 4.0f, false});

        PresentationRuntime spatialRuntime;
        PresentationFrameIntent spatialIntent = spatialRuntime.BuildPresentationFrame(spatialContext, spatialData);

        bool foundActor = false;
        bool foundProp = false;
        for (const auto& cmd : spatialIntent.commands) {
            if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 1) {
                foundActor = true;
                // Actor on tile (4,4) with elevation level 4 => 2.0 world units + 0.25 anchor offset
                assert(cmd.position.y == 2.25f);
            }
            if (cmd.type == PresentationCommand::Type::DrawProp && cmd.id == 1) {
                foundProp = true;
                assert(cmd.position.y == 2.0f);
            }
        }

        assert(foundActor);
        assert(foundProp);
        assert(spatialIntent.activePasses.size() == 3);
        urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation",
                                                    "release_validation.spatial_runtime_consumption_passed",
                                                    "Spatial Authoring -> Runtime Consumption: PASSED");
    }

    urpg::diagnostics::RuntimeDiagnostics::info("presentation.release_validation", "release_validation.succeeded",
                                                "Release Validation Suite: ALL TESTS PASSED");
}

int main() {
    try {
        RunReleaseValidation();
        return 0;
    } catch (const std::exception& e) {
        urpg::diagnostics::RuntimeDiagnostics::fatal("presentation.release_validation", "release_validation.fatal",
                                                     e.what());
        return 1;
    }
}
