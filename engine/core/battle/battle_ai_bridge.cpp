#include "battle_ai_bridge.h"
#include <sstream>

namespace urpg::ai {

std::string BattleKnowledgeBridge::describeStatBlock(const urpg::battle::BattleRuleStatBlock& stats) {
    std::stringstream ss;
    ss << "HP: " << stats.hp << "/" << stats.mhp;
    ss << ", ATK: " << stats.atk << ", DEF: " << stats.def;
    ss << ", AGI: " << stats.agi;
    if (stats.guarding) {
        ss << " (Currently Guarding)";
    }
    return ss.str();
}

std::string
BattleKnowledgeBridge::generateBattlePrompt(const urpg::battle::BattleFlowController& flow,
                                            const std::vector<EnemyIntel>& visibleEnemies,
                                            const std::vector<urpg::battle::BattleRuleStatBlock>& partyStats) {
    std::stringstream ss;
    ss << "### Current Battle State Info ###\n";
    ss << "Battle Turn: " << flow.turnCount() << "\n";
    ss << "Escape Ratio Failures: " << flow.escapeFailures() << "\n";

    ss << "\n-- Enemies --\n";
    for (const auto& enemy : visibleEnemies) {
        ss << "* " << enemy.name << " (" << enemy.description << ")\n";
        ss << "  " << describeStatBlock(enemy.stats) << "\n";
        if (!enemy.weaknesses.empty()) {
            ss << "  Weak to: ";
            for (const auto& w : enemy.weaknesses)
                ss << w << " ";
            ss << "\n";
        }
    }

    ss << "\n-- Player Party --\n";
    for (size_t i = 0; i < partyStats.size(); ++i) {
        ss << "Member " << (i + 1) << ": " << describeStatBlock(partyStats[i]) << "\n";
    }

    ss << "\nBased on the current turn, suggest a single tactical move (Attack, Guard, or Use Item) and explain WHY.";
    return ss.str();
}

} // namespace urpg::ai
