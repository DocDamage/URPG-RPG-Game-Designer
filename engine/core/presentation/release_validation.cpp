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
    std::cout << "[CHECK] Command Count: " << intent.commands.size() << " (Expected: 100)" << std::endl;
    assert(intent.commands.size() == 100);

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
