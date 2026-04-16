#pragma once

#include "engine/core/battle/battle_core.h"
#include <string>
#include <vector>
#include <memory>

namespace urpg::ai {

/**
 * @brief Enemy intelligence descriptor.
 * Tells the AI advisor what IS known vs what is HIDDEN from the player.
 */
struct EnemyIntel {
    std::string name;
    std::string description;
    urpg::battle::BattleRuleStatBlock stats;
    std::vector<std::string> weaknesses;
    std::vector<std::string> resistances;
    bool isBoss = false;
};

/**
 * @brief Bridge between the active Battle state and the AI Tactics Advisor.
 */
class BattleKnowledgeBridge {
public:
    /**
     * @brief Generates a prompt describing the CURRENT battle state.
     */
    static std::string generateBattlePrompt(
        const urpg::battle::BattleFlowController& flow,
        const std::vector<EnemyIntel>& visibleEnemies,
        const std::vector<urpg::battle::BattleRuleStatBlock>& partyStats
    );

    /**
     * @brief Translates stat blocks into natural language for the LLM.
     */
    static std::string describeStatBlock(const urpg::battle::BattleRuleStatBlock& stats);
};

} // namespace urpg::ai