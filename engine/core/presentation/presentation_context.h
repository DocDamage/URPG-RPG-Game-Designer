#pragma once

#include "presentation_types.h"
#include "presentation_schema.h"
#include "map_scene_state.h"
#include "battle_scene_state.h"

namespace urpg::presentation {

/**
 * @brief Runtime context for a single frame or session.
 * Section 9.1: PresentationContext core type
 */
struct PresentationContext {
    // Current active mode (resolved)
    PresentationMode activeMode = PresentationMode::Classic2D;

    // Current capability level
    CapabilityTier activeTier = CapabilityTier::Tier0_Baseline;

    // The current state of the game world (Section 9.1)
    MapSceneState mapState;
    BattleSceneState battleState;

    // Project settings back-reference or snapshot
    ProjectPresentationSettings projectSettings;

    // Diagnostic state for this frame/session
    bool hasWarnings = false;
    bool hasErrors = false;
};

} // namespace urpg::presentation
