#pragma once

#include "presentation_types.h"
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
};

/**
 * @brief Representation of the active battle state.
 */
struct BattleSceneState {
    std::string battleArenaId;
    std::vector<BattleParticipantState> participants;
};

} // namespace urpg::presentation
