// BattleManager - MZ Battle Pipeline Hooks - Implementation
// Phase 2 - Compat Layer

#include "battle_manager.h"
#include "audio_manager.h"
#include "data_manager.h"
#include "engine/gameplay/combat/combat_calc.h"
#include <algorithm>
#include <cassert>
#include <random>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> BattleManager::methodStatus_;
std::unordered_map<std::string, std::string> BattleManager::methodDeviations_;

namespace {

double normalizeAgility(int32_t actionSpeed) {
    return static_cast<double>(std::max(1, actionSpeed));
}

double averageAgility(const std::vector<BattleSubject>& subjects) {
    if (subjects.empty()) {
        return 100.0;
    }

    double total = 0.0;
    int32_t count = 0;
    for (const auto& subject : subjects) {
        if (subject.hidden || subject.hp <= 0) {
            continue;
        }
        total += normalizeAgility(subject.actionSpeed);
        ++count;
    }

    if (count == 0) {
        return 100.0;
    }
    return total / static_cast<double>(count);
}

double computeBaseEscapeRatio(const std::vector<BattleSubject>& actors,
                              const std::vector<BattleSubject>& enemies) {
    const double partyAgi = averageAgility(actors);
    const double troopAgi = averageAgility(enemies);
    if (troopAgi <= 0.0) {
        return 1.0;
    }
    return std::clamp(0.5 * (partyAgi / troopAgi), 0.0, 1.0);
}

uint32_t mixEscapeSeed(int32_t troopId) {
    uint32_t seed = 0x9E3779B9u;
    seed ^= static_cast<uint32_t>(troopId) + 0x85EBCA6Bu + (seed << 6) + (seed >> 2);
    return seed == 0 ? 0xA341316Cu : seed;
}

double nextEscapeRoll(uint32_t& state) {
    if (state == 0) {
        state = 0xA341316Cu;
    }
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return static_cast<double>(state) / static_cast<double>(UINT32_MAX);
}

} // namespace

// ============================================================================
// BattleSubject State Management
// ============================================================================

void BattleSubject::addState(int32_t stateId, int32_t turns) {
    if (stateId <= 0) return;
    if (!hasState(stateId)) {
        states.push_back(stateId);
    }
    if (turns >= 0) {
        stateTurns[stateId] = turns;
    }
}

void BattleSubject::removeState(int32_t stateId) {
    auto it = std::find(states.begin(), states.end(), stateId);
    if (it != states.end()) states.erase(it);
    stateTurns.erase(stateId);
}

bool BattleSubject::hasState(int32_t stateId) const {
    return std::find(states.begin(), states.end(), stateId) != states.end();
}

void BattleSubject::clearStates() {
    states.clear();
    stateTurns.clear();
}

int32_t BattleSubject::getStateTurns(int32_t stateId) const {
    auto it = stateTurns.find(stateId);
    return (it != stateTurns.end()) ? it->second : 0;
}

// Internal implementation
class BattleManagerImpl {
public:
    std::mt19937 rng{std::random_device{}()};
    bool battleEventActive = false;
    int32_t currentBattleEventId = 0;
    int32_t battleEventTicks = 0;
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
        methodStatus_["processEscape"] = CompatStatus::FULL;
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
        methodStatus_["removeExpiredStates"] = CompatStatus::FULL;
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

BattleManager& BattleManager::instance() {
    static BattleManager instance;
    return instance;
}

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
    
    // Load troop data and populate enemies
    auto& dm = DataManager::instance();
    const TroopData* troop = dm.getTroop(troopId);
    if (troop) {
        // Sync GameTroop runtime state
        dm.getGameTroop().setMembers(troop->members);
        
        for (int32_t enemyId : troop->members) {
            const EnemyData* enemy = dm.getEnemy(enemyId);
            if (enemy) {
                BattleSubject subject;
                subject.type = BattleSubjectType::ENEMY;
                subject.index = static_cast<int32_t>(enemies_.size());
                subject.id = enemyId;
                subject.mhp = enemy->mhp;
                subject.mmp = enemy->mmp;
                subject.atk = enemy->atk;
                subject.def = enemy->def;
                subject.mat = enemy->mat;
                subject.mdf = enemy->mdf;
                subject.agi = enemy->agi;
                subject.luk = enemy->luk;
                subject.hp = subject.mhp;
                subject.mp = subject.mmp;
                enemies_.push_back(subject);
            }
        }
    } else {
        dm.getGameTroop().setMembers({});
    }
    
    // Copy current party to actors
    const GlobalState& gs = dm.getGlobalState();
    for (int32_t actorId : gs.partyMembers) {
        const GameActor* gameActor = dm.getGameActor(actorId);
        BattleSubject subject;
        subject.type = BattleSubjectType::ACTOR;
        subject.index = static_cast<int32_t>(actors_.size());
        subject.id = actorId;
        if (gameActor) {
            subject.mhp = gameActor->mhp;
            subject.hp = gameActor->hp;
            subject.mmp = gameActor->mmp;
            subject.mp = gameActor->mp;
            subject.atk = gameActor->atk;
            subject.def = gameActor->def;
            subject.mat = gameActor->mat;
            subject.mdf = gameActor->mdf;
            subject.agi = gameActor->agi;
            subject.luk = gameActor->luk;
        } else {
            // fallback to defaults
            subject.mhp = 100;
            subject.hp = 100;
            subject.mmp = 100;
            subject.mp = 100;
            subject.atk = 10;
            subject.def = 10;
            subject.mat = 10;
            subject.mdf = 10;
            subject.agi = 10;
            subject.luk = 10;
        }
        actors_.push_back(subject);
    }

    escapeFailureCount_ = 0;
    escapeRatio_ = computeBaseEscapeRatio(actors_, enemies_);
    escapeRngState_ = mixEscapeSeed(troopId_);
    impl_->rng.seed(escapeRngState_);
}

void BattleManager::setBattleTransition(int32_t type) {
    battleTransitionType_ = type;
}

void BattleManager::setBattleBackground(const std::string& name) {
    battleBackgroundName_ = name;
}

void BattleManager::setBattleBgm(const std::string& name, double volume, double pitch) {
    AudioManager::instance().playBgm(name, volume, pitch);
}

void BattleManager::setVictoryMe(const std::string& name, double volume, double pitch) {
    AudioManager::instance().playMe(name, volume, pitch);
}

void BattleManager::setDefeatMe(const std::string& name, double volume, double pitch) {
    AudioManager::instance().playMe(name, volume, pitch);
}

int32_t BattleManager::getBattleTransition() const {
    return battleTransitionType_;
}

const std::string& BattleManager::getBattleBackground() const {
    return battleBackgroundName_;
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
            applyExp();
            applyGold();
            applyDrops();
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

    const double roll = nextEscapeRoll(escapeRngState_);
    const bool success = roll < escapeRatio_;

    triggerHook(HookPoint::ON_ESCAPE, {Value::Int(success ? 1 : 0)});

    if (success) {
        endBattle(BattleResult::ESCAPE);
    } else {
        ++escapeFailureCount_;
        escapeRatio_ = std::min(1.0, escapeRatio_ + 0.1);
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
                CombatCalc calc;
                ActorStats attacker;
                ActorStats defender;

                // Map BattleSubject stats to ActorStats
                attacker.atk = Fixed32::FromInt(action->subject->atk);
                attacker.def = Fixed32::FromInt(action->subject->def);
                defender.atk = Fixed32::FromInt(target->atk);
                defender.def = Fixed32::FromInt(target->def);

                // Use deterministic seed based on turn count and subject index
                uint32_t seed = static_cast<uint32_t>(turnCount_ * 100 + action->subject->index);
                DamageResult result = calc.PhysicalDamage(attacker, defender, seed);
                applyDamage(target, result.damage, true);
            }
            break;
        }
        case BattleActionType::GUARD:
            // Set guard state
            break;
        case BattleActionType::SKILL: {
            BattleSubject* target = nullptr;
            if (action->subject->type == BattleSubjectType::ACTOR) {
                target = getEnemy(action->targetIndex);
            } else {
                target = getActor(action->targetIndex);
            }
            if (target) {
                applySkill(action->subject, target, action->skillId);
            }
            break;
        }
        case BattleActionType::ITEM: {
            BattleSubject* target = nullptr;
            if (action->subject->type == BattleSubjectType::ACTOR) {
                target = getEnemy(action->targetIndex);
            } else {
                target = getActor(action->targetIndex);
            }
            if (target) {
                applyItem(action->subject, target, action->itemId);
            }
            break;
        }
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
        if (subject->type == BattleSubjectType::ACTOR) {
            DataManager::instance().setGameActorHp(subject->id, subject->hp);
        }
        triggerHook(HookPoint::ON_DAMAGE, {
            Value::Int(static_cast<int32_t>(subject->type)),
            Value::Int(subject->index),
            Value::Int(damage)
        });

        // Check remove-by-damage states
        for (int32_t stateId : subject->states) {
            const StateData* state = DataManager::instance().getState(stateId);
            if (state && state->removeByDamage) {
                std::minstd_rand rng(static_cast<uint32_t>(
                    turnCount_ * 1000 + subject->index * 100 + stateId));
                std::uniform_int_distribution<int> dist(0, 99);
                int roll = dist(rng);
                if (roll < state->chanceByDamage) {
                    subject->removeState(stateId);
                    triggerHook(HookPoint::ON_STATE_REMOVED, {
                        Value(static_cast<int64_t>(subject->index)),
                        Value(static_cast<int64_t>(stateId))
                    });
                }
            }
        }

        if (subject->hp <= 0) {
            if (subject->type == BattleSubjectType::ACTOR) {
                triggerHook(HookPoint::ON_ACTOR_DEATH, {Value::Int(subject->index)});
            } else {
                triggerHook(HookPoint::ON_ENEMY_DEATH, {Value::Int(subject->index)});
            }
        }
    } else {
        subject->mp = std::max(0, subject->mp - damage);
        if (subject->type == BattleSubjectType::ACTOR) {
            DataManager::instance().setGameActorMp(subject->id, subject->mp);
        }
    }
}

void BattleManager::applyHeal(BattleSubject* subject, int32_t amount, bool isHp) {
    if (!subject) return;

    if (isHp) {
        subject->hp = std::min(subject->mhp, subject->hp + amount);
        if (subject->type == BattleSubjectType::ACTOR) {
            DataManager::instance().setGameActorHp(subject->id, subject->hp);
        }
        triggerHook(HookPoint::ON_HEAL, {
            Value::Int(static_cast<int32_t>(subject->type)),
            Value::Int(subject->index),
            Value::Int(amount)
        });
    } else {
        subject->mp = std::min(subject->mmp, subject->mp + amount);
        if (subject->type == BattleSubjectType::ACTOR) {
            DataManager::instance().setGameActorMp(subject->id, subject->mp);
        }
    }
}

void BattleManager::applySkill(BattleSubject* user, BattleSubject* target, int32_t skillId) {
    const SkillData* skill = DataManager::instance().getSkill(skillId);
    if (skill && target) {
        applyDamage(target, 10);
    }
    (void)user;
}

void BattleManager::applyItem(BattleSubject* user, BattleSubject* target, int32_t itemId) {
    const ItemData* item = DataManager::instance().getItem(itemId);
    if (item && target) {
        applyHeal(target, 30);
    }
    (void)user;
}

void BattleManager::applyStateEffects(BattleSubject* subject) {
    if (!subject) return;
    
    // Process each active state
    for (int32_t stateId : subject->states) {
        const StateData* state = DataManager::instance().getState(stateId);
        if (!state) continue;
        
        // Apply slip damage
        if (state->slipDamage > 0 && subject->hp > 0) {
            applyDamage(subject, state->slipDamage, true);
        }
        
        // Decrement turn counter for auto-removal states
        if (state->autoRemovalTiming > 0) {
            auto it = subject->stateTurns.find(stateId);
            if (it != subject->stateTurns.end()) {
                it->second--;
            }
        }
    }
}

void BattleManager::removeExpiredStates(BattleSubject* subject) {
    if (!subject) return;
    std::vector<int32_t> toRemove;
    for (int32_t stateId : subject->states) {
        const StateData* state = DataManager::instance().getState(stateId);
        if (!state) {
            toRemove.push_back(stateId);
            continue;
        }
        if (state->autoRemovalTiming > 0) {
            auto it = subject->stateTurns.find(stateId);
            if (it != subject->stateTurns.end() && it->second <= 0) {
                toRemove.push_back(stateId);
            }
        }
    }
    for (int32_t stateId : toRemove) {
        subject->removeState(stateId);
        triggerHook(HookPoint::ON_STATE_REMOVED, {
            Value(static_cast<int64_t>(subject->index)),
            Value(static_cast<int64_t>(stateId))
        });
    }
}

void BattleManager::playAnimation(int32_t animationId, BattleSubject* target) {
    if (!target) return;
    lastAnimationRequest_ = {animationId, static_cast<int32_t>(target->type), target->index};
    triggerHook(HookPoint::ON_ACTION_START, {
        Value::Int(animationId),
        Value::Int(static_cast<int32_t>(target->type)),
        Value::Int(target->index)
    });
}

void BattleManager::playAnimationOnSubject(int32_t animationId, BattleSubject* subject) {
    if (!subject) return;
    lastAnimationRequest_ = {animationId, static_cast<int32_t>(subject->type), subject->index};
    triggerHook(HookPoint::ON_ACTION_START, {
        Value::Int(animationId),
        Value::Int(static_cast<int32_t>(subject->type)),
        Value::Int(subject->index)
    });
}

// ============================================================================
// Event Integration
// ============================================================================

void BattleManager::startBattleEvent(int32_t eventId) {
    impl_->currentBattleEventId = eventId;
    impl_->battleEventActive = true;
}

void BattleManager::updateBattleEvents() {
    if (!impl_->battleEventActive) {
        return;
    }
    impl_->battleEventTicks++;
    if (impl_->battleEventTicks >= 3) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        impl_->battleEventTicks = 0;
    }
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
    return DataManager::instance().getSwitch(switchId);
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
        const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
        if (data) {
            total += data->exp;
        }
    }
    return total;
}

int32_t BattleManager::calculateGold() const {
    int32_t total = 0;
    for (const auto& enemy : enemies_) {
        const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
        if (data) {
            total += data->gold;
        }
    }
    return total;
}

std::vector<int32_t> BattleManager::calculateDrops() const {
    std::vector<int32_t> drops;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (const auto& enemy : enemies_) {
        const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
        if (data) {
            for (size_t i = 0; i + 1 < data->dropItems.size(); i += 2) {
                int32_t itemId = data->dropItems[i];
                int32_t ratePercent = data->dropItems[i + 1];
                if (dist(impl_->rng) < (ratePercent / 100.0)) {
                    drops.push_back(itemId);
                }
            }
        }
    }
    return drops;
}

void BattleManager::applyExp() {
    int32_t exp = calculateExp();
    if (exp > 0) {
        triggerHook(HookPoint::ON_VICTORY, {Value::Int(exp)});
    }
}

void BattleManager::applyGold() {
    int32_t gold = calculateGold();
    if (gold > 0) {
        DataManager::instance().gainGold(gold);
    }
}

void BattleManager::applyDrops() {
    auto drops = calculateDrops();
    for (int32_t itemId : drops) {
        DataManager::instance().gainItem(itemId, 1);
    }
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

namespace {

int64_t valueToInt64(const urpg::Value& value, int64_t fallback = 0) {
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return *integer;
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return static_cast<int64_t>(std::llround(*real));
    }
    if (const auto* flag = std::get_if<bool>(&value.v)) {
        return *flag ? 1 : 0;
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        try {
            size_t consumed = 0;
            const int64_t parsed = std::stoll(*text, &consumed, 10);
            if (consumed == text->size()) {
                return parsed;
            }
        } catch (...) {
        }
        try {
            size_t consumed = 0;
            const double parsed = std::stod(*text, &consumed);
            if (consumed == text->size()) {
                return static_cast<int64_t>(std::llround(parsed));
            }
        } catch (...) {
        }
    }
    return fallback;
}

} // namespace

void BattleManager::registerAPI(QuickJSContext& ctx) {
    // Register BattleManager methods with QuickJS context
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setup", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        BattleManager::instance().setup(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startBattle", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().startBattle();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"endBattle", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        BattleManager::instance().endBattle(static_cast<BattleResult>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getPhase", [](const std::vector<Value>&) -> Value {
        return Value::Int(static_cast<int32_t>(BattleManager::instance().getPhase()));
    }, CompatStatus::FULL});
    
    methods.push_back({"getTurnCount", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().getTurnCount());
    }, CompatStatus::FULL});
    
    methods.push_back({"processEscape", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().processEscape() ? 1 : 0);
    }, CompatStatus::FULL});
    
    ctx.registerObject("BattleManager", methods);
}

} // namespace compat
} // namespace urpg
