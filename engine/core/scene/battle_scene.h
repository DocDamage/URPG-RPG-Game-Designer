#pragma once

#include "scene_manager.h"
#include <vector>
#include <string>

namespace urpg::scene {

/**
 * @brief Current phase of the battle.
 */
enum class BattlePhase : uint8_t {
    START,
    INPUT,
    ACTION,
    TURN_END,
    VICTORY,
    DEFEAT,
    ESCAPE
};

/**
 * @brief Native authority for the Battle flow and participant state.
 */
class BattleScene : public GameScene {
public:
    BattleScene(const std::vector<std::string>& enemyIds) 
        : m_enemyIds(enemyIds), m_currentPhase(BattlePhase::START) {}

    SceneType getType() const override { return SceneType::BATTLE; }
    std::string getName() const override { return "BattleScene"; }

    void onStart() override {
        m_currentPhase = BattlePhase::START;
        m_turnCount = 1;
    }

    BattlePhase getCurrentPhase() const { return m_currentPhase; }
    void setPhase(BattlePhase phase) { m_currentPhase = phase; }

    int getTurnCount() const { return m_turnCount; }
    void nextTurn() { m_turnCount++; }

    const std::vector<std::string>& getEnemies() const { return m_enemyIds; }

private:
    std::vector<std::string> m_enemyIds;
    BattlePhase m_currentPhase;
    int m_turnCount = 0;
};

} // namespace urpg::scene
