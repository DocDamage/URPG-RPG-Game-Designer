// BattleManager - MZ Battle Pipeline Hooks - Implementation
// Phase 2 - Compat Layer

#include "battle_manager.h"
#include <algorithm>
#include <cassert>
#include <random>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> BattleManager::methodStatus_;
std::unordered_map<std::string, std::string> BattleManager::methodDeviations_;

// Internal implementation
class BattleManagerImpl {
public:
    std::mt19937 rng{std::random_device{}()};
    bool battleEventActive = false;
    int32_t currentBattleEventId = 0;
};

BattleManager::BattleManager()
    : impl_(std::make_unique<BattleManagerImpl>())
{
    // Initialize method status registry
    if (methodStatus_.empty()) {
        methodStatus_["setup"] = CompatStatus::FULL;
        methodStatus_["setBattleTransition"] = CompatStatus::FULL;
        methodStatus_["setBattleBackground"] = CompatStatus::FULL;
        methodStatus_["setBattleBgm"] = CompatStatus::FULL;
        methodStatus_["setVictoryMe"] = CompatStatus::FULL;
        methodStatus_["setDefeatMe"] = CompatStatus::FULL;
        methodStatus_["startBattle"] = CompatStatus::FULL;
        methodStatus_["endBattle"] = CompatStatus::FULL;
        methodStatus_["abortBattle"] = CompatStatus::FULL;
        methodStatus_["canEscape"] = CompatStatus::FULL;
        methodStatus_["processEscape"] = CompatStatus::PARTIAL;
        methodDeviations_["processEscape"] = "RNG sequence may differ from MZ";
        methodStatus_["isBattleActive"] = CompatStatus::FULL;
        methodStatus_["getPhase"] = CompatStatus::FULL;
        methodStatus_["getResult"] = CompatStatus::FULL;
        methodStatus_["getTurnCount"] = CompatStatus::FULL;
        methodStatus_["incrementTurn"] = CompatStatus::FULL;
        methodStatus_["isTurn"] = CompatStatus::FULL;
        methodStatus_["isTurnRange"] = CompatStatus::FULL;
        methodStatus_["startTurn"] = CompatStatus::FULL;
        methodStatus_["endTurn"] = CompatStatus::FULL;
        methodStatus_["getActors"] = CompatStatus::FULL;
        methodStatus_["getEnemies"] = CompatStatus::FULL;
        methodStatus_["getActor"] = CompatStatus::FULL;
        methodStatus_["getEnemy"] = CompatStatus::FULL;
        methodStatus_["getAllSubjects"] = CompatStatus::FULL;
        methodStatus_["getActiveActors"] = CompatStatus::FULL;
        methodStatus_["getActiveEnemies"] = CompatStatus::FULL;
        methodStatus_["isAllActorsDead"] = CompatStatus::FULL;
        methodStatus_["isAllEnemiesDead"] = CompatStatus::FULL;
        methodStatus_["queueAction"] = CompatStatus::FULL;
        methodStatus_["forceAction"] = CompatStatus::FULL;
        methodStatus_["getNextAction"] = CompatStatus::FULL;
        methodStatus_["processAction"] = CompatStatus::FULL;
        methodStatus_["clearActions"] = CompatStatus::FULL;
        methodStatus_["sortActionsBySpeed"] = CompatStatus::FULL;
        methodStatus_["selectNextActor"] = CompatStatus::FULL;
        methodStatus_["isAllActorsInputted"] = CompatStatus::FULL;
        methodStatus_["setActorAction"] = CompatStatus::FULL;
        methodStatus_["autoBattleActor"] = CompatStatus::FULL;
        methodStatus_["applyDamage"] = CompatStatus::FULL;
        methodStatus_["applyHeal"] = CompatStatus::FULL;
        methodStatus_["applySkill"] = CompatStatus::FULL;
        methodStatus_["applyItem"] = CompatStatus::FULL;
        methodStatus_["applyStateEffects"] = CompatStatus::FULL;
        methodStatus_["playAnimation"] = CompatStatus::FULL;
        methodStatus_["playAnimationOnSubject"] = CompatStatus::FULL;
        methodStatus_["startBattleEvent"] = CompatStatus::FULL;
        methodStatus_["updateBattleEvents"] = CompatStatus::FULL;
        methodStatus_["isBattleEventActive"] = CompatStatus::FULL;
        methodStatus_["checkTurnCondition"] = CompatStatus::FULL;
        methodStatus_["checkEnemyHpCondition"] = CompatStatus::FULL;
        methodStatus_["checkActorHpCondition"] = CompatStatus::FULL;
        methodStatus_["checkSwitchCondition"] = CompatStatus::FULL;
        methodStatus_["calculateExp"] = CompatStatus::FULL;
        methodStatus_["calculateGold"] = CompatStatus::FULL;
        methodStatus_["calculateDrops"] = CompatStatus::FULL;
        methodStatus_["applyExp"] = CompatStatus::FULL;
        methodStatus_["applyGold"] = CompatStatus::FULL;
        methodStatus_["applyDrops"] = CompatStatus::FULL;
    }
}

BattleManager::~BattleManager() = default;

// ============================================================================
// Initialization and Setup
// ============================================================================

void BattleManager::setup(int32_t troopId, bool canEscape, bool canLose) {
    troopId_ = troopId;
    canEscape_ = canEscape;
    canLose_ = canLose;
    phase_ = BattlePhase::INIT;
    result_ = BattleResult::NONE;
    turnCount_ = 0;
    
    // Clear previous state
    actors_.clear();
    enemies_.clear();
    actionQueue_.clear();
    
    // Trigger hook
    triggerHook(HookPoint::ON_SETUP, {Value::Int(troopId)});
    
    // TODO: Load troop data and populate enemies
    // TODO: Copy current party to actors
}

void BattleManager::setBattleTransition(int32_t type) {
    // TODO: Set transition type for battle start/end
    (void)type;
}

void BattleManager::setBattleBackground(const std::string& name) {
    // TODO: Set battle background
    (void)name;
}

void BattleManager::setBattleBgm(const std::string& name, double volume, double pitch) {
    // TODO: Set battle BGM
    (void)name; (void)volume; (void)pitch;
}

void BattleManager::setVictoryMe(const std::string& name, double volume, double pitch) {
    // TODO: Set victory ME
    (void)name; (void)volume; (void)pitch;
}

void BattleManager::setDefeatMe(const std::string& name, double volume, double pitch) {
    // TODO: Set defeat ME
    (void)name; (void)volume; (void)pitch;
}

// ============================================================================
// Battle Flow Control
// ============================================================================

void BattleManager::startBattle() {
    phase_ = BattlePhase::START;
    triggerHook(HookPoint::ON_START, {});
    
    // Enter input phase
    phase_ = BattlePhase::INPUT;
}

void BattleManager::endBattle(BattleResult result) {
    result_ = result;
    phase_ = BattlePhase::END;
    
    switch (result) {
        case BattleResult::WIN:
            triggerHook(HookPoint::ON_VICTORY, {});
            break;
        case BattleResult::DEFEAT:
            triggerHook(HookPoint::ON_DEFEAT, {});
            break;
        case BattleResult::ESCAPE:
            triggerHook(HookPoint::ON_ESCAPE, {});
            break;
        case BattleResult::ABORT:
            triggerHook(HookPoint::ON_ABORT, {});
            break;
        default:
            break;
    }
    
    phase_ = BattlePhase::NONE;
}

void BattleManager::abortBattle() {
    endBattle(BattleResult::ABORT);
}

bool BattleManager::canEscape() const {
    return canEscape_ && phase_ == BattlePhase::INPUT;
}

bool BattleManager::processEscape() {
    if (!canEscape()) {
        return false;
    }
    
    // Calculate escape success (MZ formula approximation)
    // Escape rate = 0.5 * (party agility average / troop agility average)
    // For stub, use 50% base rate
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    bool success = dist(impl_->rng) < 0.5;
    
    triggerHook(HookPoint::ON_ESCAPE, {Value::Int(success ? 1 : 0)});
    
    if (success) {
        endBattle(BattleResult::ESCAPE);
    }
    
    return success;
}

bool BattleManager::isBattleActive() const {
    return phase_ != BattlePhase::NONE && phase_ != BattlePhase::END;
}

BattlePhase BattleManager::getPhase() const {
    return phase_;
}

BattleResult BattleManager::getResult() const {
    return result_;
}

// ============================================================================
// Turn Management
// ============================================================================

int32_t BattleManager::getTurnCount() const {
    return turnCount_;
}

void BattleManager::incrementTurn() {
    turnCount_++;
}

bool BattleManager::isTurn(int32_t turn) const {
    return turnCount_ == turn;
}

bool BattleManager::isTurnRange(int32_t minTurn, int32_t maxTurn) const {
    return turnCount_ >= minTurn && turnCount_ <= maxTurn;
}

void BattleManager::startTurn() {
    phase_ = BattlePhase::TURN;
    triggerHook(HookPoint::ON_TURN_START, {Value::Int(turnCount_)});
}

void BattleManager::endTurn() {
    triggerHook(HookPoint::ON_TURN_END, {Value::Int(turnCount_)});
    incrementTurn();
    
    // Check for battle end conditions
    if (isAllEnemiesDead()) {
        endBattle(BattleResult::WIN);
    } else if (isAllActorsDead()) {
        endBattle(canLose_ ? BattleResult::DEFEAT : BattleResult::ABORT);
    } else {
        phase_ = BattlePhase::INPUT;
    }
}

// ============================================================================
// Subject Management
// ============================================================================

std::vector<BattleSubject*> BattleManager::getActors() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        result.push_back(&actor);
    }
    return result;
}

const std::vector<BattleSubject>& BattleManager::getActorsConst() const {
    return actors_;
}

std::vector<BattleSubject*> BattleManager::getEnemies() {
    std::vector<BattleSubject*> result;
    for (auto& enemy : enemies_) {
        result.push_back(&enemy);
    }
    return result;
}

const std::vector<BattleSubject>& BattleManager::getEnemiesConst() const {
    return enemies_;
}

BattleSubject* BattleManager::getActor(int32_t index) {
    if (index < 0 || static_cast<size_t>(index) >= actors_.size()) {
        return nullptr;
    }
    return &actors_[static_cast<size_t>(index)];
}

BattleSubject* BattleManager::getEnemy(int32_t index) {
    if (index < 0 || static_cast<size_t>(index) >= enemies_.size()) {
        return nullptr;
    }
    return &enemies_[static_cast<size_t>(index)];
}

std::vector<BattleSubject*> BattleManager::getAllSubjects() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        result.push_back(&actor);
    }
    for (auto& enemy : enemies_) {
        result.push_back(&enemy);
    }
    return result;
}

std::vector<BattleSubject*> BattleManager::getActiveActors() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0) {
            result.push_back(&actor);
        }
    }
    return result;
}

std::vector<BattleSubject*> BattleManager::getActiveEnemies() {
    std::vector<BattleSubject*> result;
    for (auto& enemy : enemies_) {
        if (!enemy.hidden && enemy.hp > 0) {
            result.push_back(&enemy);
        }
    }
    return result;
}

bool BattleManager::isAllActorsDead() const {
    for (const auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0) {
            return false;
        }
    }
    return !actors_.empty();
}

bool BattleManager::isAllEnemiesDead() const {
    for (const auto& enemy : enemies_) {
        if (!enemy.hidden && enemy.hp > 0) {
            return false;
        }
    }
    return !enemies_.empty();
}

// ============================================================================
// Action System
// ============================================================================

void BattleManager::queueAction(BattleSubject* subject, BattleActionType type,
                                 int32_t targetIndex, int32_t skillId, int32_t itemId) {
    if (!subject) return;
    
    BattleAction action;
    action.subject = subject;
    action.type = type;
    action.targetIndex = targetIndex;
    action.skillId = skillId;
    action.itemId = itemId;
    action.forced = false;
    
    actionQueue_.push_back(action);
    subject->acted = true;
}

void BattleManager::forceAction(int32_t subjectIndex, BattleSubjectType subjectType,
                                 BattleActionType type, int32_t targetIndex,
                                 int32_t skillId, int32_t itemId) {
    BattleSubject* subject = nullptr;
    if (subjectType == BattleSubjectType::ACTOR) {
        subject = getActor(subjectIndex);
    } else {
        subject = getEnemy(subjectIndex);
    }
    
    if (!subject) return;
    
    BattleAction action;
    action.subject = subject;
    action.type = type;
    action.targetIndex = targetIndex;
    action.skillId = skillId;
    action.itemId = itemId;
    action.forced = true;
    
    actionQueue_.push_back(action);
}

BattleAction* BattleManager::getNextAction() {
    if (actionQueue_.empty()) {
        return nullptr;
    }
    
    currentActionIndex_ = 0;
    return &actionQueue_[0];
}

void BattleManager::processAction(BattleAction* action) {
    if (!action || !action->subject) return;
    
    phase_ = BattlePhase::ACTION;
    triggerHook(HookPoint::ON_ACTION_START, {});
    
    // Process action based on type
    switch (action->type) {
        case BattleActionType::ATTACK: {
            BattleSubject* target = nullptr;
            if (action->subject->type == BattleSubjectType::ACTOR) {
                target = getEnemy(action->targetIndex);
            } else {
                target = getActor(action->targetIndex);
            }
            if (target) {
                // TODO: Calculate and apply damage
                applyDamage(target, 10, true); // Stub damage
            }
            break;
        }
        case BattleActionType::GUARD:
            // Set guard state
            break;
        case BattleActionType::SKILL:
            // TODO: Apply skill
            break;
        case BattleActionType::ITEM:
            // TODO: Apply item
            break;
        case BattleActionType::ESCAPE:
            processEscape();
            break;
        case BattleActionType::WAIT:
            break;
    }
    
    triggerHook(HookPoint::ON_ACTION_END, {});
    
    // Remove processed action
    if (!actionQueue_.empty()) {
        actionQueue_.erase(actionQueue_.begin());
    }
    currentActionIndex_ = -1;
}

void BattleManager::clearActions() {
    actionQueue_.clear();
    currentActionIndex_ = -1;
}

void BattleManager::sortActionsBySpeed() {
    std::sort(actionQueue_.begin(), actionQueue_.end(),
              [](const BattleAction& a, const BattleAction& b) {
                  if (!a.subject || !b.subject) return false;
                  return a.subject->actionSpeed > b.subject->actionSpeed;
              });
}

// ============================================================================
// Input Phase
// ============================================================================

BattleSubject* BattleManager::selectNextActor() {
    for (auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0 && !actor.acted) {
            return &actor;
        }
    }
    return nullptr;
}

bool BattleManager::isAllActorsInputted() const {
    for (const auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0 && !actor.acted) {
            return false;
        }
    }
    return true;
}

void BattleManager::setActorAction(int32_t actorIndex, BattleActionType type,
                                    int32_t targetIndex, int32_t skillId, int32_t itemId) {
    BattleSubject* actor = getActor(actorIndex);
    if (actor) {
        queueAction(actor, type, targetIndex, skillId, itemId);
    }
}

void BattleManager::autoBattleActor(int32_t actorIndex) {
    BattleSubject* actor = getActor(actorIndex);
    if (!actor || actor->acted) return;
    
    // Simple auto-battle: attack random enemy
    auto enemies = getActiveEnemies();
    if (!enemies.empty()) {
        std::uniform_int_distribution<size_t> dist(0, enemies.size() - 1);
        int32_t targetIndex = static_cast<int32_t>(dist(impl_->rng));
        setActorAction(actorIndex, BattleActionType::ATTACK, targetIndex);
    }
}

// ============================================================================
// Damage and Effects
// ============================================================================

void BattleManager::applyDamage(BattleSubject* subject, int32_t damage, bool isHp) {
    if (!subject) return;
    
    if (isHp) {
        subject->hp = std::max(0, subject->hp - damage);
        triggerHook(HookPoint::ON_DAMAGE, {
            Value::Int(static_cast<int32_t>(subject->type)),
            Value::Int(subject->index),
            Value::Int(damage)
        });
        
        if (subject->hp <= 0) {
            if (subject->type == BattleSubjectType::ACTOR) {
                triggerHook(HookPoint::ON_ACTOR_DEATH, {Value::Int(subject->index)});
            } else {
                triggerHook(HookPoint::ON_ENEMY_DEATH, {Value::Int(subject->index)});
            }
        }
    } else {
        subject->mp = std::max(0, subject->mp - damage);
    }
}

void BattleManager::applyHeal(BattleSubject* subject, int32_t amount, bool isHp) {
    if (!subject) return;
    
    if (isHp) {
        subject->hp = std::min(subject->mhp, subject->hp + amount);
        triggerHook(HookPoint::ON_HEAL, {
            Value::Int(static_cast<int32_t>(subject->type)),
            Value::Int(subject->index),
            Value::Int(amount)
        });
    } else {
        subject->mp = std::min(subject->mmp, subject->mp + amount);
    }
}

void BattleManager::applySkill(BattleSubject* user, BattleSubject* target, int32_t skillId) {
    // TODO: Look up skill and apply effects
    (void)user; (void)target; (void)skillId;
}

void BattleManager::applyItem(BattleSubject* user, BattleSubject* target, int32_t itemId) {
    // TODO: Look up item and apply effects
    (void)user; (void)target; (void)itemId;
}

void BattleManager::applyStateEffects(BattleSubject* subject) {
    // TODO: Apply state tick effects (poison damage, regen, etc.)
    (void)subject;
}

void BattleManager::playAnimation(int32_t animationId, BattleSubject* target) {
    // TODO: Play animation on target
    (void)animationId; (void)target;
}

void BattleManager::playAnimationOnSubject(int32_t animationId, BattleSubject* subject) {
    // TODO: Play animation on subject
    (void)animationId; (void)subject;
}

// ============================================================================
// Event Integration
// ============================================================================

void BattleManager::startBattleEvent(int32_t eventId) {
    impl_->currentBattleEventId = eventId;
    impl_->battleEventActive = true;
}

void BattleManager::updateBattleEvents() {
    // TODO: Process battle event interpreter
}

bool BattleManager::isBattleEventActive() const {
    return impl_->battleEventActive;
}

bool BattleManager::checkTurnCondition(int32_t turn, int32_t span) {
    switch (span) {
        case 0: // Turn
            return isTurn(turn);
        case 1: // Turn + X
            return turnCount_ >= turn && (turnCount_ - turn) % 1 == 0;
        case 2: // Turn + Y (every Y turns after turn X)
            return turnCount_ >= turn;
        default:
            return false;
    }
}

bool BattleManager::checkEnemyHpCondition(int32_t enemyIndex, int32_t percent) {
    BattleSubject* enemy = getEnemy(enemyIndex);
    if (!enemy || enemy->mhp <= 0) return false;
    return (enemy->hp * 100 / enemy->mhp) <= percent;
}

bool BattleManager::checkActorHpCondition(int32_t actorIndex, int32_t percent) {
    BattleSubject* actor = getActor(actorIndex);
    if (!actor || actor->mhp <= 0) return false;
    return (actor->hp * 100 / actor->mhp) <= percent;
}

bool BattleManager::checkSwitchCondition(int32_t switchId) {
    // TODO: Check game switch
    (void)switchId;
    return false;
}

// ============================================================================
// Hook Registration
// ============================================================================

void BattleManager::registerHook(HookPoint point, const std::string& pluginId, BattleHookFn callback) {
    hooks_[point][pluginId] = std::move(callback);
}

void BattleManager::unregisterHooks(const std::string& pluginId) {
    for (auto& [point, pluginMap] : hooks_) {
        pluginMap.erase(pluginId);
    }
}

Value BattleManager::triggerHook(HookPoint point, const std::vector<Value>& args) {
    auto pointIt = hooks_.find(point);
    if (pointIt == hooks_.end()) {
        return Value::Nil();
    }
    
    // Call all registered hooks for this point
    for (auto& [pluginId, callback] : pointIt->second) {
        callback(args);
    }
    
    return Value::Nil();
}

// ============================================================================
// Drop/Exp/Gold
// ============================================================================

int32_t BattleManager::calculateExp() const {
    int32_t total = 0;
    for (const auto& enemy : enemies_) {
        // TODO: Get base exp from enemy database entry
        total += 10; // Stub
    }
    return total;
}

int32_t BattleManager::calculateGold() const {
    int32_t total = 0;
    for (const auto& enemy : enemies_) {
        // TODO: Get base gold from enemy database entry
        total += 5; // Stub
    }
    return total;
}

std::vector<int32_t> BattleManager::calculateDrops() const {
    std::vector<int32_t> drops;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (const auto& enemy : enemies_) {
        // TODO: Check drop rates from enemy database entry
        if (dist(impl_->rng) < 0.1) { // 10% drop chance stub
            drops.push_back(1); // Stub item ID
        }
    }
    return drops;
}

void BattleManager::applyExp() {
    int32_t exp = calculateExp();
    // TODO: Distribute EXP to party
    (void)exp;
}

void BattleManager::applyGold() {
    int32_t gold = calculateGold();
    // TODO: Add gold to party
    (void)gold;
}

void BattleManager::applyDrops() {
    auto drops = calculateDrops();
    // TODO: Add items to party inventory
    (void)drops;
}

// ============================================================================
// Compat Status
// ============================================================================

CompatStatus BattleManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string BattleManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void BattleManager::registerAPI(QuickJSContext& ctx) {
    // Register BattleManager methods with QuickJS context
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setup", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        // BattleManager::instance().setup(args[0].asInt());
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startBattle", [](const std::vector<Value>&) -> Value {
        // BattleManager::instance().startBattle();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"endBattle", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        // BattleManager::instance().endBattle(static_cast<BattleResult>(args[0].asInt()));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getPhase", [](const std::vector<Value>&) -> Value {
        // return Value::Int(static_cast<int32_t>(BattleManager::instance().getPhase()));
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"getTurnCount", [](const std::vector<Value>&) -> Value {
        // return Value::Int(BattleManager::instance().getTurnCount());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"processEscape", [](const std::vector<Value>&) -> Value {
        // return Value::Int(BattleManager::instance().processEscape() ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::PARTIAL, "RNG sequence may differ from MZ"});
    
    ctx.registerObject("BattleManager", methods);
}

} // namespace compat
} // namespace urpg
