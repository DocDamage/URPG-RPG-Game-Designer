#pragma once

#include "effects/effect_cue.h"
#include "presentation_types.h"
#include <cstdint>
#include <vector>
#include <string>

namespace urpg::presentation {

/**
 * @brief Represents a participant in a battle (hero or enemy).
 */
struct BattleParticipantState {
    uint32_t actorId;
    std::string classId;
    int32_t formationIndex; // Position index within the formation
    bool isEnemy;
    float currentHPPulse;   // Visual feedback for health
    std::uint64_t cueId = 0;
    Vec3 anchorPosition = {0.0f, 0.0f, 0.0f};
    bool hasAnchorPosition = false;
};

/**
 * @brief Representation of the active battle state.
 */
struct BattleSceneState {
    std::string battleArenaId;
    std::vector<BattleParticipantState> participants;
    std::vector<effects::EffectCue> effectCues;
};

} // namespace urpg::presentation
