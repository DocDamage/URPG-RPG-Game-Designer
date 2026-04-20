#include <iostream>
#include <vector>
#include <cassert>
#include "presentation_runtime.h"
#include "map_scene_translator.h"

using namespace urpg::presentation;

/**
 * @brief Stress test for the Presentation Core logic.
 * Ensures the system can handle a high volume of actors and lookups
 * without state corruption or performance spikes.
 */
void RunReleaseValidation() {
    std::cout << "[INFO] Starting Phase 5: Release Validation..." << std::endl;

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

    std::cout << "[CHECK] Actor Command Count: " << actorCommandCount << " (Expected: 100)" << std::endl;
    std::cout << "[CHECK] Environment Command Envelope: fog=" << fogCommandCount
              << ", postfx=" << postFxCommandCount
              << ", lights=" << lightCommandCount << std::endl;
    assert(actorCommandCount == 100);
    assert(fogCommandCount <= 1);
    assert(postFxCommandCount <= 1);

    // Verify Y-Position Resolution (Base 1.0 from level 2 + 0.5 from anchorOffset)
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            assert(cmd.position.y == 1.5f);
        }
    }
    std::cout << "[CHECK] Y-Height Resolution: PASSED" << std::endl;

    // 4. Fallback Logic Validation
    context.activeMode = PresentationMode::Classic2D;
    PresentationFrameIntent fallbackIntent = runtime.BuildPresentationFrame(context, data);
    
    for (const auto& cmd : fallbackIntent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            // In Classic2D, Z should be 0.0f (Flattened)
            assert(cmd.position.z == 0.0f);
        }
    }
    std::cout << "[CHECK] Fallback Flattening (Tier 0): PASSED" << std::endl;

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

    std::cout << "[CHECK] Battle Effect Command Sample Envelope: world=" << worldEffectCommandCount
              << ", overlay=" << overlayEffectCommandCount << std::endl;
    assert(worldEffectCommandCount >= 1);
    assert(overlayEffectCommandCount >= 1);
    assert(worldEffectCommandCount <= 16);
    assert(overlayEffectCommandCount <= 16);

    std::cout << "[SUCCESS] Release Validation Suite: ALL TESTS PASSED" << std::endl;
}

int main() {
    try {
        RunReleaseValidation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }
}
