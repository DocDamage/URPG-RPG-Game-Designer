#pragma once

// BattleManager - MZ Battle Pipeline Hooks
// Phase 2 - Compat Layer
//
// This defines the BattleManager compatibility surface for MZ battle plugins.
// Per Section 4 - WindowCompat Explicit Surface:
// BattleManager pipeline hooks are required for battle visual plugins.
//
// The BattleManager provides hook points at each phase of the MZ battle flow,
// allowing plugins to intercept and modify battle behavior.

#include "quickjs_runtime.h"
#include "engine/runtimes/bridge/value.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class BattleManagerImpl;

// Battle phases matching MZ flow
enum class BattlePhase : uint8_t {
    NONE = 0,
    INIT = 1,
    START = 2,
    INPUT = 3,
    TURN = 4,
    ACTION = 5,
    END = 6,
    ABORT = 7
};

// Battle action types
enum class BattleActionType : uint8_t {
    ATTACK = 0,
    GUARD = 1,
    SKILL = 2,
    ITEM = 3,
    ESCAPE = 4,
    WAIT = 5
};

// Subject types in battle
enum class BattleSubjectType : uint8_t {
    ACTOR = 0,
    ENEMY = 1
};

struct BattleStateEffect {
    int32_t stateId = 0;
    int32_t turnsRemaining = 0;
    int32_t hpDeltaPerTurn = 0;
    int32_t mpDeltaPerTurn = 0;
};

struct BattleModifierEffect {
    int32_t paramId = 0;
    int32_t stages = 0;
    int32_t turnsRemaining = 0;
};

// Battle subject reference (actor or enemy)
struct BattleSubject {
    BattleSubjectType type = BattleSubjectType::ACTOR;
    int32_t index = 0;          // Index in actor/enemy array
    int32_t id = 0;             // Database ID
    
    // Current state
    int32_t hp = 0;
    int32_t mp = 0;
    int32_t tp = 0;
    int32_t mhp = 0;
    int32_t mmp = 0;
    
    // Battle state
    bool hidden = false;
    bool immortal = false;
    bool acted = false;
    int32_t actionSpeed = 0;
    
    // Pending action
    BattleActionType pendingAction = BattleActionType::WAIT;
    int32_t targetIndex = -1;   // -1 = no target, -2 = random
    int32_t skillId = 0;
    int32_t itemId = 0;
    std::vector<BattleStateEffect> states;
    std::vector<BattleModifierEffect> modifiers;
};

// Battle action being executed
struct BattleAction {
    BattleSubject* subject = nullptr;
    BattleActionType type = BattleActionType::ATTACK;
    int32_t targetIndex = -1;
    int32_t skillId = 0;
    int32_t itemId = 0;
    int32_t animationId = 0;
    bool forced = false;
};

// Battle result
enum class BattleResult : uint8_t {
    NONE = 0,
    WIN = 1,
    ESCAPE = 2,
    DEFEAT = 3,
    ABORT = 4
};

// Hook function types
using BattleHookFn = std::function<Value(const std::vector<Value>& args)>;

// BattleManager - MZ compatibility layer for battle system
//
// This class provides the MZ BattleManager API surface with hook points
// at each battle phase. Plugins can register callbacks to modify behavior.
//
class BattleManager {
public:
    BattleManager();
    ~BattleManager();
    
    // Non-copyable
    BattleManager(const BattleManager&) = delete;
    BattleManager& operator=(const BattleManager&) = delete;

    // Singleton access for compatibility.
    static BattleManager& instance();
    
    // ========================================================================
    // Initialization and Setup
    // ========================================================================
    
    // Status: PARTIAL - Battle state initializes, but troop loading and party seeding are still TODO
    void setup(int32_t troopId, bool canEscape = true, bool canLose = false);
    
    // Status: STUB - Transition type is accepted but not routed to runtime output
    void setBattleTransition(int32_t type);
    
    // Status: STUB - Background selection is accepted but not routed to runtime output
    void setBattleBackground(const std::string& name);
    
    // Status: STUB - Audio metadata is accepted but not routed to playback
    void setBattleBgm(const std::string& name, double volume = 90.0, double pitch = 100.0);
    void setVictoryMe(const std::string& name, double volume = 90.0, double pitch = 100.0);
    void setDefeatMe(const std::string& name, double volume = 90.0, double pitch = 100.0);
    
    // ========================================================================
    // Battle Flow Control
    // ========================================================================
    
    // Status: FULL - Start battle
    void startBattle();
    
    // Status: FULL - End battle
    void endBattle(BattleResult result);
    
    // Status: FULL - Abort battle
    void abortBattle();
    
    // Status: FULL - Escape attempt with deterministic MZ-style ratio ramp
    bool canEscape() const;
    bool processEscape();
    
    // Status: FULL - Check if battle is active
    bool isBattleActive() const;
    
    // Status: FULL - Get current phase
    BattlePhase getPhase() const;
    
    // Status: FULL - Get battle result
    BattleResult getResult() const;
    
    // ========================================================================
    // Turn Management
    // ========================================================================
    
    // Status: FULL - Turn counter
    int32_t getTurnCount() const;
    void incrementTurn();
    
    // Status: FULL - Check if specific turn (for event conditions)
    bool isTurn(int32_t turn) const;
    bool isTurnRange(int32_t minTurn, int32_t maxTurn) const;
    
    // Status: FULL - Turn end processing
    void startTurn();
    void endTurn();
    
    // ========================================================================
    // Subject (Actor/Enemy) Management
    // ========================================================================
    
    // Status: FULL - Get all actors in battle
    std::vector<BattleSubject*> getActors();
    const std::vector<BattleSubject>& getActorsConst() const;
    
    // Status: FULL - Get all enemies in battle
    std::vector<BattleSubject*> getEnemies();
    const std::vector<BattleSubject>& getEnemiesConst() const;
    
    // Status: FULL - Get specific subject
    BattleSubject* getActor(int32_t index);
    BattleSubject* getEnemy(int32_t index);
    void addActorSubject(const BattleSubject& subject);
    void addEnemySubject(const BattleSubject& subject);
    
    // Status: FULL - Get all battle members (actors + enemies)
    std::vector<BattleSubject*> getAllSubjects();
    
    // Status: FULL - Get active subjects (not hidden/dead)
    std::vector<BattleSubject*> getActiveActors();
    std::vector<BattleSubject*> getActiveEnemies();
    
    // Status: FULL - Check if party/ troop is all dead
    bool isAllActorsDead() const;
    bool isAllEnemiesDead() const;
    
    // ========================================================================
    // Action System
    // ========================================================================
    
    // Status: FULL - Queue action for subject
    void queueAction(BattleSubject* subject, BattleActionType type,
                     int32_t targetIndex = -1, int32_t skillId = 0, int32_t itemId = 0);
    
    // Status: FULL - Force action (ignores input phase)
    void forceAction(int32_t subjectIndex, BattleSubjectType subjectType,
                     BattleActionType type, int32_t targetIndex,
                     int32_t skillId = 0, int32_t itemId = 0);
    
    // Status: FULL - Get next action to process
    BattleAction* getNextAction();
    
    // Status: FULL - Process action
    void processAction(BattleAction* action);
    
    // Status: FULL - Clear all actions
    void clearActions();
    
    // Status: FULL - Sort actions by speed
    void sortActionsBySpeed();
    
    // ========================================================================
    // Input Phase
    // ========================================================================
    
    // Status: FULL - Select next actor for input
    BattleSubject* selectNextActor();
    
    // Status: FULL - Check if all actors have input
    bool isAllActorsInputted() const;
    
    // Status: FULL - Set actor action
    void setActorAction(int32_t actorIndex, BattleActionType type,
                        int32_t targetIndex, int32_t skillId = 0, int32_t itemId = 0);
    
    // Status: FULL - Auto battle for actor
    void autoBattleActor(int32_t actorIndex);
    
    // ========================================================================
    // Damage and Effects
    // ========================================================================
    
    // Status: FULL - Apply damage to subject
    void applyDamage(BattleSubject* subject, int32_t damage, bool isHp = true);
    
    // Status: FULL - Apply healing to subject
    void applyHeal(BattleSubject* subject, int32_t amount, bool isHp = true);
    
    // Status: STUB - Placeholder path; does not resolve skill database effects
    void applySkill(BattleSubject* user, BattleSubject* target, int32_t skillId);
    
    // Status: STUB - Placeholder path; does not resolve item database effects
    void applyItem(BattleSubject* user, BattleSubject* target, int32_t itemId);

    // Status: FULL - Add/remove/query state effects
    bool addState(BattleSubject* subject, int32_t stateId, int32_t turnsRemaining = 1,
                  int32_t hpDeltaPerTurn = 0, int32_t mpDeltaPerTurn = 0);
    bool removeState(BattleSubject* subject, int32_t stateId);
    bool hasState(const BattleSubject* subject, int32_t stateId) const;
    bool addBuff(BattleSubject* subject, int32_t paramId, int32_t turnsRemaining = 1, int32_t stages = 1);
    bool addDebuff(BattleSubject* subject, int32_t paramId, int32_t turnsRemaining = 1, int32_t stages = 1);
    int32_t getModifierStage(const BattleSubject* subject, int32_t paramId) const;
    
    // Status: FULL - Check and apply states
    void applyStateEffects(BattleSubject* subject);
    void applyTurnEndEffects(BattleSubject* subject);
    
    // Status: STUB - Animation intent is recorded, but playback is still TODO
    void playAnimation(int32_t animationId, BattleSubject* target);
    void playAnimationOnSubject(int32_t animationId, BattleSubject* subject);
    
    // ========================================================================
    // Event Integration
    // ========================================================================
    
    // Status: PARTIAL - Event state toggles exist, but interpreter execution is still TODO
    void startBattleEvent(int32_t eventId);
    void updateBattleEvents();
    bool isBattleEventActive() const;
    
    // Status: PARTIAL/STUB - Turn and HP checks are live; switch checks still fall back
    bool checkTurnCondition(int32_t turn, int32_t span);
    bool checkEnemyHpCondition(int32_t enemyIndex, int32_t percent);
    bool checkActorHpCondition(int32_t actorIndex, int32_t percent);
    bool checkSwitchCondition(int32_t switchId);
    
    // ========================================================================
    // Hook Registration
    // ========================================================================
    
    // Hook points for plugins
    enum class HookPoint : uint8_t {
        ON_SETUP,               // Before battle setup
        ON_START,               // Battle starts
        ON_TURN_START,          // Turn begins
        ON_TURN_END,            // Turn ends
        ON_ACTION_START,        // Action begins
        ON_ACTION_END,          // Action ends
        ON_DAMAGE,              // Damage dealt
        ON_HEAL,                // Healing applied
        ON_STATE_ADDED,         // State added to subject
        ON_STATE_REMOVED,       // State removed from subject
        ON_ACTOR_DEATH,         // Actor dies
        ON_ENEMY_DEATH,         // Enemy dies
        ON_VICTORY,             // Victory
        ON_DEFEAT,              // Defeat
        ON_ESCAPE,              // Escape attempt
        ON_ABORT                // Battle aborted
    };
    
    // Register a hook callback
    void registerHook(HookPoint point, const std::string& pluginId, BattleHookFn callback);
    
    // Unregister hooks for a plugin
    void unregisterHooks(const std::string& pluginId);
    
    // Trigger a hook (internal)
    Value triggerHook(HookPoint point, const std::vector<Value>& args);
    
    // ========================================================================
    // Drop/Exp/Gold
    // ========================================================================
    
    // Status: PARTIAL - Reward math still relies on seeded subject data and stub drops
    int32_t calculateExp() const;
    int32_t calculateGold() const;
    std::vector<int32_t> calculateDrops() const;
    
    // Status: STUB - Reward application into party progression/inventory is still TODO
    void applyExp();
    void applyGold();
    void applyDrops();
    
    // ========================================================================
    // Compat Status
    // ========================================================================
    
    // Register BattleManager API with QuickJS context
    static void registerAPI(QuickJSContext& ctx);
    
    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);
    
private:
    std::unique_ptr<BattleManagerImpl> impl_;
    
    // Battle state
    int32_t troopId_ = 0;
    BattlePhase phase_ = BattlePhase::NONE;
    BattleResult result_ = BattleResult::NONE;
    int32_t turnCount_ = 0;
    bool canEscape_ = true;
    bool canLose_ = false;
    double escapeRatio_ = 0.5;
    int32_t escapeFailureCount_ = 0;
    uint32_t escapeRngState_ = 0;
    
    // Subjects
    std::vector<BattleSubject> actors_;
    std::vector<BattleSubject> enemies_;
    
    // Actions
    std::vector<BattleAction> actionQueue_;
    int32_t currentActionIndex_ = -1;
    
    // Hooks
    std::unordered_map<HookPoint, std::unordered_map<std::string, BattleHookFn>> hooks_;
    
    // API status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;
};

} // namespace compat
} // namespace urpg
