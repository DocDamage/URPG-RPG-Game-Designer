#pragma once

#include "scene_manager.h"
#include "engine/core/math/vector2.h"
#include "engine/core/ui/ui_window.h"
#include "engine/core/ui/ui_command_list.h"
#include "engine/core/render/sprite_animator.h"
#include <vector>
#include <string>
#include <memory>

namespace urpg::scene {

/**
 * @brief Represents a participant in the battle (Actor or Enemy).
 */
struct BattleParticipant {
    std::string id;
    std::string name;
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    Vector2f position;
    std::unique_ptr<urpg::SpriteAnimator> animator;
    bool isEnemy;
        
        // Phase 9: Visual FX
        float DamagePopupValue = 0;
        float DamagePopupTimer = 0;
        uint32_t DamagePopupColor = 0xFFFFFFFF;
        
        // Status Effects
        std::vector<int32_t> states; // IDs from states.json
        bool isGuarding = false;
    };

enum class BattlePhase {
    START = 0,
    INPUT,
    ACTION,
    TURN_END,
    VICTORY,
    DEFEAT
};

/**
 * @brief Native authority for the Battle flow and participant state.
 */
class BattleScene : public GameScene {
public:
    BattleScene(const std::vector<std::string>& enemyIds);

    SceneType getType() const override { return SceneType::BATTLE; }
    std::string getName() const override { return "BattleScene"; }

    void onStart() override;
    void onUpdate(float dt) override;
    void draw(urpg::SpriteBatcher& batcher) override;

    BattlePhase getCurrentPhase() const { return m_currentPhase; }
    void setPhase(BattlePhase phase) { m_currentPhase = phase; }

    int getTurnCount() const { return m_turnCount; }
    void nextTurn() { m_turnCount++; }

    const std::vector<std::string>& getEnemies() const { return m_enemyIds; }

    /**
     * @brief Setup the battle from a specific troop ID.
     */
    void setupTroop(int32_t troopId);

    void addActor(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos, std::shared_ptr<urpg::Texture> texture);
    void addEnemy(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos, std::shared_ptr<urpg::Texture> texture);

    struct BattleAction {
        BattleParticipant* subject;
        BattleParticipant* target; // Singular target (for single-target actions)
        std::vector<BattleParticipant*> multiTargets; // Phase 10: For AoE actions
        std::string command; // "attack", "skill", "item", "guard"
        int32_t skillId = -1; // ID if skill command
        int32_t itemId = -1; // ID if item command
        bool isSkill = false;
        bool isItem = false;
        bool isAoE = false; // Phase 10: Multi-target flag
    };

protected:
    void onCommandSelected(const std::string& cmd);
    void onTargetSelected(BattleParticipant* target);
    void onMultiTargetSelected(bool targetEnemies); // Phase 10
    void openSkillWindow();
    void openItemWindow();
    void openTargetWindow(bool targetEnemies);
    
    void processTurn();
    void executeAction(const BattleAction& action);

    /**
     * @brief Processes battle rewards (EXP, Gold, Items) after victory.
     */
    void processVictoryRewards();

    /**
     * @brief Checks if all enemies or all actors are defeated.
     * @return BattlePhase::VICTORY, BattlePhase::DEFEAT, or BattlePhase::ACTION to continue
     */
    BattlePhase checkEndCondition();

public:
    // Testing access
    const std::vector<BattleParticipant>& getParticipants() const { return m_participants; }
    void addActionToQueue(const BattleAction& action) { m_actionQueue.push_back(action); }

private:
    std::vector<std::string> m_enemyIds;
    BattlePhase m_currentPhase;
    int m_turnCount = 0;
    float m_phaseTimer = 0.0f;
    float m_shakeTimer = 0.0f; // Screen shake for hits
    size_t m_currentActorIndex = 0;

    // Temporary action being built
    BattleAction m_pendingAction;

    std::vector<BattleParticipant> m_participants;
    std::vector<BattleAction> m_actionQueue;

    // Background
    std::shared_ptr<urpg::Texture> m_backgroundTexture;

    // UI elements
    std::shared_ptr<ui::UIWindow> m_logWindow;
    std::shared_ptr<ui::UIWindow> m_statusWindow;
    std::shared_ptr<ui::UICommandList> m_commandWindow;
    std::shared_ptr<ui::UICommandList> m_skillWindow; // Skill selection
    std::shared_ptr<ui::UICommandList> m_itemWindow;  // Item selection
    std::shared_ptr<ui::UICommandList> m_targetWindow; // Enemy/Actor selection
};

} // namespace urpg::scene
