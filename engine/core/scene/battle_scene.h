#pragma once

#include "engine/core/ability/ability_system_component.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/math/vector2.h"
#include "engine/core/presentation/effects/effect_cue.h"
#include "engine/core/render/sprite_animator.h"
#include "engine/core/ui/ui_command_list.h"
#include "engine/core/ui/ui_window.h"
#include "scene_manager.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::scene {

/**
 * @brief Represents a participant in the battle (Actor or Enemy).
 */
struct BattleParticipant {
    struct ModifierEffect {
        int32_t paramId = 0;
        int32_t stages = 0;
        int32_t turnsRemaining = 0;
    };

    std::string id;
    std::string name;
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    Vector2f position;
    std::unique_ptr<urpg::SpriteAnimator> animator;
    bool isEnemy;
    urpg::ability::AbilitySystemComponent abilitySystem;

    // Phase 9: Visual FX
    float DamagePopupValue = 0;
    float DamagePopupTimer = 0;
    uint32_t DamagePopupColor = 0xFFFFFFFF;

    // Status Effects
    std::vector<int32_t> states; // IDs from states.json
    std::vector<ModifierEffect> modifiers;
    bool isGuarding = false;
};

enum class BattlePhase { START = 0, INPUT, ACTION, TURN_END, VICTORY, DEFEAT };

struct BattleDiagnosticsPreview {
    urpg::battle::BattleDamageContext physical_preview;
    urpg::battle::BattleDamageContext magical_preview;
    int32_t party_agi = 0;
    int32_t troop_agi = 0;
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
    void setPhase(BattlePhase phase);

    int getTurnCount() const { return m_turnCount; }
    void nextTurn() { m_turnCount++; }

    const std::vector<std::string>& getEnemies() const { return m_enemyIds; }

    /**
     * @brief Setup the battle from a specific troop ID.
     */
    void setupTroop(int32_t troopId);

    void addActor(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos,
                  std::shared_ptr<urpg::Texture> texture);
    void addEnemy(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos,
                  std::shared_ptr<urpg::Texture> texture);

    struct BattleAction {
        BattleParticipant* subject;
        BattleParticipant* target;                    // Singular target (for single-target actions)
        std::vector<BattleParticipant*> multiTargets; // Phase 10: For AoE actions
        std::string command;                          // "attack", "skill", "item", "guard"
        int32_t skillId = -1;                         // ID if skill command
        int32_t itemId = -1;                          // ID if item command
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
    void addActionToQueue(const BattleAction& action);
    urpg::battle::BattleFlowController& flowController() { return m_flowController; }
    const urpg::battle::BattleFlowController& flowController() const { return m_flowController; }
    urpg::battle::BattleActionQueue& nativeActionQueue() { return m_nativeActionQueue; }
    const urpg::battle::BattleActionQueue& nativeActionQueue() const { return m_nativeActionQueue; }
    void enqueueEffectCue(const urpg::presentation::effects::EffectCue& cue);
    const std::vector<urpg::presentation::effects::EffectCue>& effectCues() const { return m_effectCues; }
    void clearEffectCues() { m_effectCues.clear(); }
    std::optional<BattleDiagnosticsPreview> buildDiagnosticsPreview() const;

  private:
    urpg::battle::BattleQueuedAction makeQueuedAction(const BattleAction& action) const;
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
    urpg::battle::BattleFlowController m_flowController;
    urpg::battle::BattleActionQueue m_nativeActionQueue;
    uint32_t m_effectSequence = 0;
    std::vector<urpg::presentation::effects::EffectCue> m_effectCues;

    // Background
    std::shared_ptr<urpg::Texture> m_backgroundTexture;

    // UI elements
    std::shared_ptr<ui::UIWindow> m_logWindow;
    std::shared_ptr<ui::UIWindow> m_statusWindow;
    std::shared_ptr<ui::UICommandList> m_commandWindow;
    std::shared_ptr<ui::UICommandList> m_skillWindow;  // Skill selection
    std::shared_ptr<ui::UICommandList> m_itemWindow;   // Item selection
    std::shared_ptr<ui::UICommandList> m_targetWindow; // Enemy/Actor selection
};

} // namespace urpg::scene
