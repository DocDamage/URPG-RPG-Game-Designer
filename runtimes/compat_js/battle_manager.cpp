// BattleManager - MZ Battle Pipeline Hooks - Implementation
// Phase 2 - Compat Layer

#include "battle_manager.h"
#include "data_manager.h"
#include <algorithm>
#include <cassert>
#include <random>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> BattleManager::methodStatus_;
std::unordered_map<std::string, std::string> BattleManager::methodDeviations_;

namespace {

Value battleAudioCueToValue(const BattleAudioCue& cue) {
    Object obj;
    obj["name"].v = cue.name;
    obj["volume"].v = cue.volume;
    obj["pitch"].v = cue.pitch;
    return Value::Obj(std::move(obj));
}

const AnimationData* findAnimationData(int32_t animationId) {
    if (animationId <= 0) {
        return nullptr;
    }

    const auto& animations = DataManager::instance().getAnimations();
    if (animations.empty()) {
        return nullptr;
    }

    const size_t index = static_cast<size_t>(animationId - 1);
    if (index < animations.size() && animations[index].id == animationId) {
        return &animations[index];
    }

    for (const auto& animation : animations) {
        if (animation.id == animationId) {
            return &animation;
        }
    }

    return nullptr;
}

int32_t resolveAnimationDurationFrames(int32_t animationId) {
    const AnimationData* animation = findAnimationData(animationId);
    if (!animation) {
        return 0;
    }

    if (!animation->frames.empty()) {
        return std::max(1, static_cast<int32_t>(animation->frames.size()) * 4);
    }

    constexpr int32_t baseFrames = 24;
    constexpr int32_t stepFrames = 6;
    constexpr int32_t variationCount = 5;
    return baseFrames + ((animationId % variationCount) * stepFrames);
}

const Object* asObject(const Value& value) {
    return std::get_if<Object>(&value.v);
}

const Array* asArray(const Value& value) {
    return std::get_if<Array>(&value.v);
}

int32_t valueToInt(const Value& value, int32_t fallback = 0) {
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        return static_cast<int32_t>(*asInt);
    }
    if (const auto* asDouble = std::get_if<double>(&value.v)) {
        return static_cast<int32_t>(*asDouble);
    }
    if (const auto* asBool = std::get_if<bool>(&value.v)) {
        return *asBool ? 1 : 0;
    }
    return fallback;
}

bool valueToBool(const Value& value, bool fallback = false) {
    if (const auto* asBool = std::get_if<bool>(&value.v)) {
        return *asBool;
    }
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        return *asInt != 0;
    }
    return fallback;
}

std::string valueToString(const Value& value, const std::string& fallback = "") {
    if (const auto* asString = std::get_if<std::string>(&value.v)) {
        return *asString;
    }
    return fallback;
}

BattleStateEffect* findStateEffect(BattleSubject* subject, int32_t stateId) {
    if (!subject) {
        return nullptr;
    }

    auto it = std::find_if(subject->states.begin(), subject->states.end(),
                           [stateId](const BattleStateEffect& effect) {
                               return effect.stateId == stateId;
                           });
    if (it == subject->states.end()) {
        return nullptr;
    }
    return &(*it);
}

BattleModifierEffect* findModifierEffect(BattleSubject* subject, int32_t paramId) {
    if (!subject) {
        return nullptr;
    }

    auto it = std::find_if(subject->modifiers.begin(), subject->modifiers.end(),
                           [paramId](const BattleModifierEffect& effect) {
                               return effect.paramId == paramId;
                           });
    if (it == subject->modifiers.end()) {
        return nullptr;
    }
    return &(*it);
}

int32_t clampModifierStage(int32_t stage) {
    return std::clamp(stage, -2, 2);
}

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

constexpr int32_t kAttackParamId = 2;
constexpr int32_t kDefenseParamId = 3;
constexpr int32_t kRecoveryParamId = 4;
constexpr int32_t kAgilityParamId = 6;

int32_t getModifierStage(const BattleSubject* subject, int32_t paramId) {
    if (!subject || paramId < 0) {
        return 0;
    }

    const auto it = std::find_if(subject->modifiers.begin(), subject->modifiers.end(),
                                 [paramId](const BattleModifierEffect& effect) {
                                     return effect.paramId == paramId;
                                 });
    if (it == subject->modifiers.end()) {
        return 0;
    }

    return it->stages;
}

double modifierMultiplier(int32_t stage) {
    if (stage >= 0) {
        return 1.0 + (0.25 * static_cast<double>(stage));
    }
    return 1.0 / (1.0 + (0.25 * static_cast<double>(-stage)));
}

int32_t scaleMagnitude(int32_t amount, int32_t stage) {
    return std::max(0, static_cast<int32_t>(std::lround(static_cast<double>(amount) * modifierMultiplier(stage))));
}

int32_t resolveAttackDamage(const BattleSubject* subject, const BattleSubject* target, int32_t baseDamage) {
    const double attackMultiplier = modifierMultiplier(getModifierStage(subject, kAttackParamId));
    const double defenseMultiplier = modifierMultiplier(getModifierStage(target, kDefenseParamId));
    return std::max(1, static_cast<int32_t>(std::lround(
        static_cast<double>(baseDamage) * (attackMultiplier / defenseMultiplier))));
}

double targetPriorityScore(const BattleSubject* target) {
    const int32_t missingHp = std::max(0, target->mhp - target->hp);
    const int32_t defenseStage = getModifierStage(target, kDefenseParamId);
    const int32_t agilityStage = getModifierStage(target, kAgilityParamId);
    return static_cast<double>(missingHp) +
           (4.0 * static_cast<double>(-defenseStage)) +
           (2.0 * static_cast<double>(-agilityStage));
}

BattleSubject* resolveActionTarget(BattleManager* manager, const BattleAction& action) {
    if (!action.subject || !manager) {
        return nullptr;
    }

    if (action.subject->type == BattleSubjectType::ACTOR) {
        if (action.targetIndex >= 0) {
            return manager->getEnemy(action.targetIndex);
        }
        auto enemies = manager->getActiveEnemies();
        if (enemies.empty()) {
            return nullptr;
        }

        return *std::max_element(enemies.begin(), enemies.end(),
                                 [](const BattleSubject* lhs, const BattleSubject* rhs) {
                                     return targetPriorityScore(lhs) < targetPriorityScore(rhs);
                                 });
    }

    if (action.targetIndex >= 0) {
        return manager->getActor(action.targetIndex);
    }

    auto actors = manager->getActiveActors();
    if (actors.empty()) {
        return nullptr;
    }

    return *std::max_element(actors.begin(), actors.end(),
                             [](const BattleSubject* lhs, const BattleSubject* rhs) {
                                 return targetPriorityScore(lhs) < targetPriorityScore(rhs);
                             });
}

template <typename Fn>
int32_t accumulateRewardFromEligibleEnemies(const std::vector<BattleSubject>& enemies, Fn&& rewardForEnemy) {
    const bool hasDefeatedEnemy = std::any_of(
        enemies.begin(),
        enemies.end(),
        [](const BattleSubject& enemy) { return enemy.hp <= 0; }
    );

    int32_t total = 0;
    for (const auto& enemy : enemies) {
        if (hasDefeatedEnemy) {
            if (enemy.hp > 0) {
                continue;
            }
        } else if (enemy.hidden) {
            continue;
        }

        total += rewardForEnemy(enemy);
    }

    return total;
}

void syncActorSubjectToDataManager(const BattleSubject* subject) {
    if (!subject || subject->type != BattleSubjectType::ACTOR) {
        return;
    }

    auto* actor = DataManager::instance().getActor(subject->id);
    if (!actor) {
        return;
    }

    actor->hp = std::clamp(subject->hp, 0, subject->mhp);
    actor->mp = std::clamp(subject->mp, 0, subject->mmp);
    actor->tp = std::clamp(subject->tp, 0, 100);
}

constexpr int32_t kVictorySwitchId = 101;
constexpr int32_t kDefeatSwitchId = 102;
constexpr int32_t kEscapeSwitchId = 103;

} // namespace

// Internal implementation
class BattleManagerImpl {
public:
    struct BattleEventPageState {
        bool ranThisBattle = false;
        int32_t lastTriggeredTurn = -1;
        int64_t lastTriggeredTick = -1;
    };

    mutable std::mt19937 rng{std::random_device{}()};
    bool battleEventActive = false;
    int32_t currentBattleEventId = 0;
    bool manualActorsOverride = false;
    bool manualEnemiesOverride = false;
    int64_t battleEventTick = 0;
    std::vector<BattleEventPageState> pageStates;
};

BattleManager::BattleManager()
    : impl_(std::make_unique<BattleManagerImpl>())
{
    // Initialize method status registry
    if (methodStatus_.empty()) {
        const auto setStatus = [](const std::string& method,
                                  CompatStatus status,
                                  const std::string& deviation = "") {
            methodStatus_[method] = status;
            if (deviation.empty()) {
                methodDeviations_.erase(method);
            } else {
                methodDeviations_[method] = deviation;
            }
        };

        setStatus("setup", CompatStatus::PARTIAL,
                  "Troop setup and party seeding are implemented, but rely on DataManager database loaders which are still partial. Enemy positioning is omitted because BattleSubject has no position fields.");
        setStatus("setBattleTransition", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("setBattleBackground", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("setBattleBgm", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("setVictoryMe", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("setDefeatMe", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("changeBattleBackground", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("changeBattleBgm", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("changeVictoryMe", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
        setStatus("changeDefeatMe", CompatStatus::PARTIAL,
                  "Transition/background/audio metadata is retained for compat readback, but scene/audio backend routing is still TODO.");
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
        methodStatus_["addActorSubject"] = CompatStatus::FULL;
        methodStatus_["addEnemySubject"] = CompatStatus::FULL;
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
        setStatus("applySkill", CompatStatus::PARTIAL,
                  "Resolves skill database record and applies damage/healing/state effects. Full formula parsing is not yet implemented.");
        setStatus("applyItem", CompatStatus::PARTIAL,
                  "Resolves item database record and applies damage/healing/state effects. Full formula parsing is not yet implemented.");
        methodStatus_["addState"] = CompatStatus::FULL;
        methodStatus_["removeState"] = CompatStatus::FULL;
        methodStatus_["hasState"] = CompatStatus::FULL;
        methodStatus_["addBuff"] = CompatStatus::FULL;
        methodStatus_["addDebuff"] = CompatStatus::FULL;
        methodStatus_["getModifierStage"] = CompatStatus::FULL;
        methodStatus_["applyStateEffects"] = CompatStatus::FULL;
        methodStatus_["applyTurnEndEffects"] = CompatStatus::FULL;
        methodStatus_["playAnimation"] = CompatStatus::FULL;
        methodStatus_["playAnimationOnSubject"] = CompatStatus::FULL;
        setStatus("startBattleEvent", CompatStatus::PARTIAL,
                  "Troop page conditions and a bounded command subset execute against live compat state, but full MZ battle interpreter coverage is still TODO.");
        setStatus("updateBattleEvents", CompatStatus::PARTIAL,
                  "Battle-event updates advance deterministic animations and run troop page conditions plus a bounded command subset, but full MZ battle interpreter coverage is still TODO.");
        methodStatus_["isBattleEventActive"] = CompatStatus::FULL;
        methodStatus_["checkTurnCondition"] = CompatStatus::FULL;
        methodStatus_["checkEnemyHpCondition"] = CompatStatus::FULL;
        methodStatus_["checkActorHpCondition"] = CompatStatus::FULL;
        methodStatus_["checkSwitchCondition"] = CompatStatus::FULL;
        setStatus("calculateExp", CompatStatus::PARTIAL,
                  "Reward math now queries enemy database, but DataManager enemy loader is still partial.");
        setStatus("calculateGold", CompatStatus::PARTIAL,
                  "Reward math now queries enemy database, but DataManager enemy loader is still partial.");
        setStatus("calculateDrops", CompatStatus::PARTIAL,
                  "Drop logic now queries enemy database and rolls probabilities, but DataManager enemy loader is still partial.");
        setStatus("applyExp", CompatStatus::PARTIAL,
                  "Real EXP progression with level-up and skill learning. Uses simplified exp table rather than full MZ formula curve.");
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
    DataManager& dm = DataManager::instance();
    if (dm.getActors().empty() && dm.getEnemies().empty() && dm.getTroops().empty()) {
        dm.loadDatabase();
    }
    if (dm.getPartySize() == 0 && dm.getStartPartySize() > 0) {
        dm.setupNewGame();
    }

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
    impl_->manualActorsOverride = false;
    impl_->manualEnemiesOverride = false;
    impl_->battleEventActive = false;
    impl_->currentBattleEventId = 0;
    impl_->battleEventTick = 0;
    impl_->pageStates.clear();
    
    // Trigger hook
    triggerHook(HookPoint::ON_SETUP, {Value::Int(troopId)});
    
    // Load troop data and populate enemies
    const TroopData* troop = DataManager::instance().getTroop(troopId_);
    if (troop) {
        if (const auto* pages = asArray(troop->pages)) {
            impl_->pageStates.resize(pages->size());
        }
        for (int32_t enemyId : troop->members) {
            const EnemyData* enemyData = DataManager::instance().getEnemy(enemyId);
            if (enemyData) {
                BattleSubject enemy;
                enemy.type = BattleSubjectType::ENEMY;
                enemy.index = static_cast<int32_t>(enemies_.size());
                enemy.id = enemyData->id;
                enemy.hp = enemyData->mhp;
                enemy.mhp = enemyData->mhp;
                enemy.mp = enemyData->mmp;
                enemy.mmp = enemyData->mmp;
                enemy.tp = 0;
                enemy.actionSpeed = enemyData->agi;
                enemy.hidden = false;
                enemy.immortal = false;
                enemy.acted = false;
                enemies_.push_back(enemy);
            }
        }
    }

    // Copy current party to actors
    const auto& party = dm.getGlobalState().partyMembers;
    for (int32_t actorId : party) {
        const ActorData* actorData = dm.getActor(actorId);
        if (actorData) {
            BattleSubject actor;
            actor.type = BattleSubjectType::ACTOR;
            actor.index = static_cast<int32_t>(actors_.size());
            actor.id = actorData->id;
            int32_t level = actorData->level;
            if (level < 1) level = 1;
            if (!actorData->params.empty() && static_cast<size_t>(level) <= actorData->params.size()) {
                const auto& params = actorData->params[static_cast<size_t>(level - 1)];
                actor.mhp = params.size() > 0 ? params[0] : 100;
                actor.mmp = params.size() > 1 ? params[1] : 100;
                actor.actionSpeed = params.size() > 6 ? params[6] : 100;
            } else {
                actor.mhp = 100;
                actor.mmp = 100;
                actor.actionSpeed = 100;
            }
            actor.hp = std::clamp(actorData->hp, 0, actor.mhp);
            actor.mp = std::clamp(actorData->mp, 0, actor.mmp);
            actor.tp = std::clamp(actorData->tp, 0, 100);
            actor.hidden = false;
            actor.immortal = false;
            actor.acted = false;
            actors_.push_back(actor);
        }
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
    battleBackground_ = name;
}

void BattleManager::setBattleBgm(const std::string& name, double volume, double pitch) {
    battleBgm_.name = name;
    battleBgm_.volume = std::clamp(volume, 0.0, 100.0);
    battleBgm_.pitch = std::clamp(pitch, 50.0, 200.0);
}

void BattleManager::setVictoryMe(const std::string& name, double volume, double pitch) {
    victoryMe_.name = name;
    victoryMe_.volume = std::clamp(volume, 0.0, 100.0);
    victoryMe_.pitch = std::clamp(pitch, 50.0, 200.0);
}

void BattleManager::setDefeatMe(const std::string& name, double volume, double pitch) {
    defeatMe_.name = name;
    defeatMe_.volume = std::clamp(volume, 0.0, 100.0);
    defeatMe_.pitch = std::clamp(pitch, 50.0, 200.0);
}

int32_t BattleManager::getBattleTransition() const {
    return battleTransitionType_;
}

const std::string& BattleManager::getBattleBackground() const {
    return battleBackground_;
}

const BattleAudioCue& BattleManager::getBattleBgm() const {
    return battleBgm_;
}

const BattleAudioCue& BattleManager::getVictoryMe() const {
    return victoryMe_;
}

const BattleAudioCue& BattleManager::getDefeatMe() const {
    return defeatMe_;
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
    DataManager& dm = DataManager::instance();
    
    switch (result) {
        case BattleResult::WIN:
            dm.setSwitch(kVictorySwitchId, true);
            triggerHook(HookPoint::ON_VICTORY, {});
            break;
        case BattleResult::DEFEAT:
            dm.setSwitch(kDefeatSwitchId, true);
            triggerHook(HookPoint::ON_DEFEAT, {});
            break;
        case BattleResult::ESCAPE:
            dm.setSwitch(kEscapeSwitchId, true);
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
    for (auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0) {
            applyTurnEndEffects(&actor);
        }
    }
    for (auto& enemy : enemies_) {
        if (!enemy.hidden && enemy.hp > 0) {
            applyTurnEndEffects(&enemy);
        }
    }

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

void BattleManager::addActorSubject(const BattleSubject& subject) {
    if (!impl_->manualActorsOverride) {
        actors_.clear();
        impl_->manualActorsOverride = true;
    }
    actors_.push_back(subject);
}

void BattleManager::addEnemySubject(const BattleSubject& subject) {
    if (!impl_->manualEnemiesOverride) {
        enemies_.clear();
        impl_->manualEnemiesOverride = true;
    }
    enemies_.push_back(subject);
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
            BattleSubject* target = resolveActionTarget(this, *action);
            if (target) {
                applyDamage(target, resolveAttackDamage(action->subject, target, 10), true);
            }
            break;
        }
        case BattleActionType::GUARD:
            // Set guard state
            break;
        case BattleActionType::SKILL: {
            BattleSubject* target = resolveActionTarget(this, *action);
            if (target) {
                applySkill(action->subject, target, action->skillId);
            }
            break;
        }
        case BattleActionType::ITEM: {
            BattleSubject* target = resolveActionTarget(this, *action);
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
    
    auto enemies = getActiveEnemies();
    if (!enemies.empty()) {
        const auto bestTarget = *std::max_element(enemies.begin(), enemies.end(),
                                                  [](const BattleSubject* lhs, const BattleSubject* rhs) {
                                                      return targetPriorityScore(lhs) < targetPriorityScore(rhs);
                                                  });
        int32_t targetIndex = bestTarget->index;
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
        syncActorSubjectToDataManager(subject);
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
        syncActorSubjectToDataManager(subject);
    }
}

void BattleManager::applyHeal(BattleSubject* subject, int32_t amount, bool isHp) {
    if (!subject) return;

    const int32_t resolvedAmount = scaleMagnitude(amount, getModifierStage(subject, kRecoveryParamId));
    
    if (isHp) {
        subject->hp = std::min(subject->mhp, subject->hp + resolvedAmount);
        syncActorSubjectToDataManager(subject);
        triggerHook(HookPoint::ON_HEAL, {
            Value::Int(static_cast<int32_t>(subject->type)),
            Value::Int(subject->index),
            Value::Int(resolvedAmount)
        });
    } else {
        subject->mp = std::min(subject->mmp, subject->mp + resolvedAmount);
        syncActorSubjectToDataManager(subject);
    }
}

void BattleManager::applySkill(BattleSubject* user, BattleSubject* target, int32_t skillId) {
    if (!target) return;
    const SkillData* skill = DataManager::instance().getSkill(skillId);
    if (!skill) return;
    if (skill->animationId > 0) {
        playAnimation(skill->animationId, target);
    }
    const int32_t dmgType = skill->damage.type;
    if (dmgType == 1 || dmgType == 2) {
        int32_t amount = resolveAttackDamage(user, target, skill->damage.power);
        applyDamage(target, amount, dmgType == 1);
    } else if (dmgType == 3 || dmgType == 4) {
        applyHeal(target, skill->damage.power, dmgType == 3);
    }
    for (const auto& eff : skill->effects) {
        if (eff.code == 11) {
            applyHeal(target, static_cast<int32_t>(eff.value2), true);
        } else if (eff.code == 12) {
            applyHeal(target, static_cast<int32_t>(eff.value2), false);
        } else if (eff.code == 21) {
            addState(target, eff.dataId, 3);
        } else if (eff.code == 22) {
            removeState(target, eff.dataId);
        }
    }
}

void BattleManager::applyItem(BattleSubject* user, BattleSubject* target, int32_t itemId) {
    if (!target) return;
    const ItemData* item = DataManager::instance().getItem(itemId);
    if (!item) return;
    if (item->animationId > 0) {
        playAnimation(item->animationId, target);
    }
    const int32_t dmgType = item->damage.type;
    if (dmgType == 1 || dmgType == 2) {
        int32_t amount = resolveAttackDamage(user, target, item->damage.power);
        applyDamage(target, amount, dmgType == 1);
    } else if (dmgType == 3 || dmgType == 4) {
        applyHeal(target, item->damage.power, dmgType == 3);
    }
    for (const auto& eff : item->effects) {
        if (eff.code == 11) {
            applyHeal(target, static_cast<int32_t>(eff.value2), true);
        } else if (eff.code == 12) {
            applyHeal(target, static_cast<int32_t>(eff.value2), false);
        } else if (eff.code == 21) {
            addState(target, eff.dataId, 3);
        } else if (eff.code == 22) {
            removeState(target, eff.dataId);
        }
    }
}

bool BattleManager::addState(BattleSubject* subject, int32_t stateId, int32_t turnsRemaining,
                             int32_t hpDeltaPerTurn, int32_t mpDeltaPerTurn) {
    if (!subject || stateId <= 0) {
        return false;
    }

    BattleStateEffect* existing = findStateEffect(subject, stateId);
    if (existing != nullptr) {
        existing->turnsRemaining = std::max(0, turnsRemaining);
        existing->hpDeltaPerTurn = hpDeltaPerTurn;
        existing->mpDeltaPerTurn = mpDeltaPerTurn;
        return true;
    }

    BattleStateEffect effect;
    effect.stateId = stateId;
    effect.turnsRemaining = std::max(0, turnsRemaining);
    effect.hpDeltaPerTurn = hpDeltaPerTurn;
    effect.mpDeltaPerTurn = mpDeltaPerTurn;
    subject->states.push_back(effect);

    triggerHook(HookPoint::ON_STATE_ADDED, {
        Value::Int(static_cast<int32_t>(subject->type)),
        Value::Int(subject->index),
        Value::Int(stateId)
    });
    return true;
}

bool BattleManager::removeState(BattleSubject* subject, int32_t stateId) {
    if (!subject || stateId <= 0) {
        return false;
    }

    const auto it = std::find_if(subject->states.begin(), subject->states.end(),
                                 [stateId](const BattleStateEffect& effect) {
                                     return effect.stateId == stateId;
                                 });
    if (it == subject->states.end()) {
        return false;
    }

    subject->states.erase(it);
    triggerHook(HookPoint::ON_STATE_REMOVED, {
        Value::Int(static_cast<int32_t>(subject->type)),
        Value::Int(subject->index),
        Value::Int(stateId)
    });
    return true;
}

bool BattleManager::hasState(const BattleSubject* subject, int32_t stateId) const {
    if (!subject || stateId <= 0) {
        return false;
    }

    return std::any_of(subject->states.begin(), subject->states.end(),
                       [stateId](const BattleStateEffect& effect) {
                           return effect.stateId == stateId;
                       });
}

bool BattleManager::addBuff(BattleSubject* subject, int32_t paramId, int32_t turnsRemaining, int32_t stages) {
    if (!subject || paramId < 0 || stages <= 0) {
        return false;
    }

    BattleModifierEffect* existing = findModifierEffect(subject, paramId);
    if (existing != nullptr) {
        existing->stages = clampModifierStage(existing->stages + stages);
        existing->turnsRemaining = std::max(existing->turnsRemaining, std::max(0, turnsRemaining));
        return true;
    }

    BattleModifierEffect effect;
    effect.paramId = paramId;
    effect.stages = clampModifierStage(stages);
    effect.turnsRemaining = std::max(0, turnsRemaining);
    subject->modifiers.push_back(effect);
    return true;
}

bool BattleManager::addDebuff(BattleSubject* subject, int32_t paramId, int32_t turnsRemaining, int32_t stages) {
    if (!subject || paramId < 0 || stages <= 0) {
        return false;
    }

    BattleModifierEffect* existing = findModifierEffect(subject, paramId);
    if (existing != nullptr) {
        existing->stages = clampModifierStage(existing->stages - stages);
        existing->turnsRemaining = std::max(existing->turnsRemaining, std::max(0, turnsRemaining));
        return true;
    }

    BattleModifierEffect effect;
    effect.paramId = paramId;
    effect.stages = clampModifierStage(-stages);
    effect.turnsRemaining = std::max(0, turnsRemaining);
    subject->modifiers.push_back(effect);
    return true;
}

int32_t BattleManager::getModifierStage(const BattleSubject* subject, int32_t paramId) const {
    return compat::getModifierStage(subject, paramId);
}

void BattleManager::applyStateEffects(BattleSubject* subject) {
    if (!subject) {
        return;
    }

    std::vector<int32_t> expiredStates;
    for (auto& effect : subject->states) {
        if (effect.hpDeltaPerTurn > 0) {
            applyHeal(subject, effect.hpDeltaPerTurn, true);
        } else if (effect.hpDeltaPerTurn < 0) {
            applyDamage(subject, -effect.hpDeltaPerTurn, true);
        }

        if (effect.mpDeltaPerTurn > 0) {
            applyHeal(subject, effect.mpDeltaPerTurn, false);
        } else if (effect.mpDeltaPerTurn < 0) {
            applyDamage(subject, -effect.mpDeltaPerTurn, false);
        }

        if (effect.turnsRemaining > 0) {
            --effect.turnsRemaining;
            if (effect.turnsRemaining == 0) {
                expiredStates.push_back(effect.stateId);
            }
        }
    }

    for (int32_t stateId : expiredStates) {
        removeState(subject, stateId);
    }
}

void BattleManager::applyTurnEndEffects(BattleSubject* subject) {
    if (!subject) {
        return;
    }

    applyStateEffects(subject);

    auto it = subject->modifiers.begin();
    while (it != subject->modifiers.end()) {
        if (it->turnsRemaining > 0) {
            --it->turnsRemaining;
        }

        if (it->turnsRemaining == 0 || it->stages == 0) {
            it = subject->modifiers.erase(it);
            continue;
        }
        ++it;
    }
}

void BattleManager::playAnimation(int32_t animationId, BattleSubject* target) {
    if (!target) {
        return;
    }

    const int32_t durationFrames = resolveAnimationDurationFrames(animationId);
    if (durationFrames <= 0) {
        return;
    }

    BattleAnimationPlayback playback;
    playback.animationId = animationId;
    playback.targetType = target->type;
    playback.targetIndex = target->index;
    playback.targetId = target->id;
    playback.framesRemaining = durationFrames;
    playback.subjectAnimation = false;
    activeAnimations_.push_back(std::move(playback));
}

void BattleManager::playAnimationOnSubject(int32_t animationId, BattleSubject* subject) {
    if (!subject) {
        return;
    }

    const int32_t durationFrames = resolveAnimationDurationFrames(animationId);
    if (durationFrames <= 0) {
        return;
    }

    BattleAnimationPlayback playback;
    playback.animationId = animationId;
    playback.targetType = subject->type;
    playback.targetIndex = subject->index;
    playback.targetId = subject->id;
    playback.framesRemaining = durationFrames;
    playback.subjectAnimation = true;
    activeAnimations_.push_back(std::move(playback));
}

bool BattleManager::isAnimationPlaying() const {
    return !activeAnimations_.empty();
}

const std::vector<BattleAnimationPlayback>& BattleManager::getActiveAnimations() const {
    return activeAnimations_;
}

// ============================================================================
// Event Integration
// ============================================================================

void BattleManager::startBattleEvent(int32_t eventId) {
    const TroopData* troop = DataManager::instance().getTroop(troopId_);
    if (!troop) {
        impl_->currentBattleEventId = 0;
        impl_->battleEventActive = false;
        return;
    }

    const auto* pages = asArray(troop->pages);
    if (!pages || eventId <= 0 || static_cast<size_t>(eventId) > pages->size()) {
        impl_->currentBattleEventId = 0;
        impl_->battleEventActive = false;
        return;
    }

    impl_->currentBattleEventId = eventId;
    impl_->battleEventActive = true;
}

void BattleManager::updateBattleEvents() {
    ++impl_->battleEventTick;

    for (auto& animation : activeAnimations_) {
        if (animation.framesRemaining > 0) {
            --animation.framesRemaining;
        }
    }

    activeAnimations_.erase(
        std::remove_if(activeAnimations_.begin(), activeAnimations_.end(),
                       [](const BattleAnimationPlayback& animation) {
                           return animation.framesRemaining <= 0;
                       }),
        activeAnimations_.end());

    const TroopData* troop = DataManager::instance().getTroop(troopId_);
    const auto* pages = troop ? asArray(troop->pages) : nullptr;
    if (!pages || pages->empty()) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    if (impl_->pageStates.size() != pages->size()) {
        impl_->pageStates.resize(pages->size());
    }

    const auto findActorIndexById = [this](int32_t actorId) -> int32_t {
        for (size_t i = 0; i < actors_.size(); ++i) {
            if (actors_[i].id == actorId) {
                return static_cast<int32_t>(i);
            }
        }
        return -1;
    };

    const auto isPageConditionMet = [this, &findActorIndexById](const Object& pageObject) -> bool {
        const auto conditionsIt = pageObject.find("conditions");
        if (conditionsIt == pageObject.end()) {
            return true;
        }

        const auto* conditions = asObject(conditionsIt->second);
        if (!conditions) {
            return true;
        }

        if (valueToBool(conditions->at("turnValid"), false)) {
            const int32_t turnA = valueToInt(conditions->at("turnA"), 0);
            const int32_t turnB = valueToInt(conditions->at("turnB"), 0);
            if (!checkTurnCondition(turnA, turnB)) {
                return false;
            }
        }

        if (valueToBool(conditions->at("switchValid"), false)) {
            if (!checkSwitchCondition(valueToInt(conditions->at("switchId"), 0))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("enemyValid"), false)) {
            if (!checkEnemyHpCondition(valueToInt(conditions->at("enemyIndex"), 0),
                                       valueToInt(conditions->at("enemyHp"), 100))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("actorValid"), false)) {
            const int32_t actorIndex = findActorIndexById(valueToInt(conditions->at("actorId"), 0));
            if (actorIndex < 0 ||
                !checkActorHpCondition(actorIndex, valueToInt(conditions->at("actorHp"), 100))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("turnEnding"), false) && phase_ != BattlePhase::TURN) {
            return false;
        }

        return true;
    };

    if (!impl_->battleEventActive) {
        for (size_t pageIndex = 0; pageIndex < pages->size(); ++pageIndex) {
            const auto* pageObject = asObject((*pages)[pageIndex]);
            if (!pageObject || !isPageConditionMet(*pageObject)) {
                continue;
            }

            const int32_t span = valueToInt(pageObject->at("span"), 0);
            auto& pageState = impl_->pageStates[pageIndex];
            bool shouldTrigger = false;
            if (span == 0) {
                shouldTrigger = !pageState.ranThisBattle;
            } else if (span == 1) {
                shouldTrigger = pageState.lastTriggeredTurn != turnCount_;
            } else {
                shouldTrigger = pageState.lastTriggeredTick != impl_->battleEventTick;
            }

            if (shouldTrigger) {
                startBattleEvent(static_cast<int32_t>(pageIndex + 1));
                pageState.ranThisBattle = true;
                pageState.lastTriggeredTurn = turnCount_;
                pageState.lastTriggeredTick = impl_->battleEventTick;
                break;
            }
        }
    }

    if (!impl_->battleEventActive) {
        return;
    }

    const size_t activePageIndex = static_cast<size_t>(impl_->currentBattleEventId - 1);
    if (activePageIndex >= pages->size()) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    const auto* pageObject = asObject((*pages)[activePageIndex]);
    if (!pageObject) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    const auto listIt = pageObject->find("list");
    const auto* commands = (listIt != pageObject->end()) ? asArray(listIt->second) : nullptr;
    if (!commands) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    std::vector<bool> branchStack;
    const auto isParentBranchActive = [&branchStack]() -> bool {
        return std::all_of(branchStack.begin(), branchStack.end(), [](bool active) { return active; });
    };
    const auto evaluateConditional = [this](const Array& params) -> bool {
        if (params.empty()) {
            return false;
        }

        const int32_t mode = valueToInt(params[0], -1);
        if (mode == 0 && params.size() >= 3) {
            const int32_t switchId = valueToInt(params[1], 0);
            const bool expected = valueToInt(params[2], 0) == 0;
            return DataManager::instance().getSwitch(switchId) == expected;
        }

        if (mode == 12 && params.size() >= 2) {
            return valueToString(params[1]) == "BattleManager.isBattleTest()" && isBattleTest();
        }

        return false;
    };

    for (const auto& commandValue : *commands) {
        const auto* command = asObject(commandValue);
        if (!command) {
            continue;
        }

        const int32_t code = valueToInt(command->at("code"), 0);
        const auto paramsIt = command->find("parameters");
        const Array emptyParams;
        const Array* params = (paramsIt != command->end()) ? asArray(paramsIt->second) : &emptyParams;

        if (code == 111) {
            const bool parentActive = isParentBranchActive();
            const bool conditionMet = parentActive && params && evaluateConditional(*params);
            branchStack.push_back(conditionMet);
            continue;
        }

        if (code == 411) {
            if (!branchStack.empty()) {
                const bool previous = branchStack.back();
                branchStack.pop_back();
                const bool parentActive = isParentBranchActive();
                branchStack.push_back(parentActive && !previous);
            }
            continue;
        }

        if (code == 412) {
            if (!branchStack.empty()) {
                branchStack.pop_back();
            }
            continue;
        }

        if (!isParentBranchActive()) {
            continue;
        }

        switch (code) {
            case 0:
            case 108:
            case 408:
                break;
            case 121:
                if (params && params->size() >= 3) {
                    const int32_t startId = valueToInt((*params)[0], 0);
                    const int32_t endId = valueToInt((*params)[1], startId);
                    const bool value = valueToInt((*params)[2], 0) == 0;
                    for (int32_t switchId = startId; switchId <= endId; ++switchId) {
                        DataManager::instance().setSwitch(switchId, value);
                    }
                }
                break;
            case 122:
                if (params && params->size() >= 5) {
                    const int32_t startId = valueToInt((*params)[0], 0);
                    const int32_t endId = valueToInt((*params)[1], startId);
                    const int32_t operation = valueToInt((*params)[2], 0);
                    const int32_t operandType = valueToInt((*params)[3], 0);
                    const int32_t operand = (operandType == 0) ? valueToInt((*params)[4], 0) : 0;
                    for (int32_t variableId = startId; variableId <= endId; ++variableId) {
                        int32_t current = DataManager::instance().getVariable(variableId);
                        switch (operation) {
                            case 0: current = operand; break;
                            case 1: current += operand; break;
                            case 2: current -= operand; break;
                            case 3: current *= operand; break;
                            case 4: if (operand != 0) current /= operand; break;
                            case 5: if (operand != 0) current %= operand; break;
                            default: break;
                        }
                        DataManager::instance().setVariable(variableId, current);
                    }
                }
                break;
            case 125:
                if (params && params->size() >= 3) {
                    const int32_t operation = valueToInt((*params)[0], 0);
                    const int32_t operandType = valueToInt((*params)[1], 0);
                    const int32_t amount = (operandType == 0) ? valueToInt((*params)[2], 0) : 0;
                    if (operation == 0) {
                        DataManager::instance().gainGold(amount);
                    } else {
                        DataManager::instance().loseGold(amount);
                    }
                }
                break;
            default:
                break;
        }
    }

    impl_->battleEventActive = false;
    impl_->currentBattleEventId = 0;
}

bool BattleManager::isBattleEventActive() const {
    return impl_->battleEventActive;
}

bool BattleManager::checkTurnCondition(int32_t turn, int32_t span) {
    if (span < 0) {
        return false;
    }

    if (span == 0) {
        return isTurn(turn);
    }

    if (turnCount_ < turn) {
        return false;
    }

    return (turnCount_ - turn) % span == 0;
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
    return accumulateRewardFromEligibleEnemies(enemies_, [](const BattleSubject& enemy) {
        const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
        return data ? data->exp : 10;
    });
}

int32_t BattleManager::calculateGold() const {
    return accumulateRewardFromEligibleEnemies(enemies_, [](const BattleSubject& enemy) {
        const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
        return data ? data->gold : 5;
    });
}

std::vector<int32_t> BattleManager::calculateDrops() const {
    std::vector<int32_t> drops;
    for (const auto& enemy : enemies_) {
        if (enemy.hp <= 0) {
            const EnemyData* data = DataManager::instance().getEnemy(enemy.id);
            if (!data || data->dropItems.empty()) continue;
            // dropItems layout: [itemId, rate, itemId, rate, ...]
            for (size_t i = 0; i + 1 < data->dropItems.size(); i += 2) {
                int32_t itemId = data->dropItems[i];
                int32_t rate = data->dropItems[i + 1];
                if (rate <= 0) continue;
                std::uniform_int_distribution<int> dist(1, rate);
                if (dist(impl_->rng) == 1) {
                    drops.push_back(itemId);
                }
            }
        }
    }
    return drops;
}

void BattleManager::applyExp() {
    int32_t exp = calculateExp();
    if (exp <= 0) return;
    DataManager& dm = DataManager::instance();
    for (int32_t i = 0; i < dm.getPartySize(); ++i) {
        int32_t actorId = dm.getPartyMember(i);
        if (actorId > 0) {
            dm.gainExp(actorId, exp);
        }
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
    DataManager& dm = DataManager::instance();
    for (int32_t itemId : drops) {
        dm.gainItem(itemId, 1);
    }
}

void BattleManager::seedRng(uint32_t seed) {
    impl_->rng.seed(seed);
}

// ============================================================================
// JS-friendly helpers
// ============================================================================

bool BattleManager::isBattleTest() const {
    return false;
}

bool BattleManager::canLose() const {
    return canLose_;
}

void BattleManager::onEscapeSuccess() {
    triggerHook(HookPoint::ON_ESCAPE, {Value::Int(1)});
}

void BattleManager::onEscapeFailure() {
    ++escapeFailureCount_;
    escapeRatio_ = std::min(1.0, escapeRatio_ + 0.1);
}

void BattleManager::changeBattleBackground(const std::string& name) {
    setBattleBackground(name);
}

void BattleManager::changeBattleBgm(const std::string& name, double volume, double pitch) {
    setBattleBgm(name, volume, pitch);
}

void BattleManager::changeVictoryMe(const std::string& name, double volume, double pitch) {
    setVictoryMe(name, volume, pitch);
}

void BattleManager::changeDefeatMe(const std::string& name, double volume, double pitch) {
    setDefeatMe(name, volume, pitch);
}

bool BattleManager::isStateActive(const BattleSubject* subject, int32_t stateId) const {
    return hasState(subject, stateId);
}

void BattleManager::processAction() {
    BattleAction* action = getNextAction();
    if (action) {
        processAction(action);
    }
}

void BattleManager::queueActionByIndices(int32_t subjectIndex, BattleSubjectType subjectType,
                                         BattleActionType type, int32_t targetIndex,
                                         int32_t skillId, int32_t itemId) {
    BattleSubject* subject = nullptr;
    if (subjectType == BattleSubjectType::ACTOR) {
        subject = getActor(subjectIndex);
    } else {
        subject = getEnemy(subjectIndex);
    }
    if (subject) {
        queueAction(subject, type, targetIndex, skillId, itemId);
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

void BattleManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setup", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        int32_t troopId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) troopId = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        bool canEscape = true;
        bool canLose = false;
        if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) canEscape = std::get<int64_t>(args[1].v) != 0;
        if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) canLose = std::get<int64_t>(args[2].v) != 0;
        BattleManager::instance().setup(troopId, canEscape, canLose);
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"startBattle", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().startBattle();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"abortBattle", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().abortBattle();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"endBattle", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        int32_t result = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) result = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        BattleManager::instance().endBattle(static_cast<BattleResult>(result));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"isBattleTest", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().isBattleTest() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"canEscape", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().canEscape() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"canLose", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().canLose() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"onEscapeSuccess", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().onEscapeSuccess();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"onEscapeFailure", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().onEscapeFailure();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"changeBattleBackground", [](const std::vector<Value>& args) -> Value {
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) {
            BattleManager::instance().changeBattleBackground(std::get<std::string>(args[0].v));
        }
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"changeBattleBgm", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeBattleBgm(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"changeVictoryMe", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeVictoryMe(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"changeDefeatMe", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeDefeatMe(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL});

    methods.push_back({"getBattleTransition", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().getBattleTransition());
    }, CompatStatus::PARTIAL});

    methods.push_back({"getBattleBackground", [](const std::vector<Value>&) -> Value {
        Value background;
        background.v = BattleManager::instance().getBattleBackground();
        return background;
    }, CompatStatus::PARTIAL});

    methods.push_back({"getBattleBgm", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getBattleBgm());
    }, CompatStatus::PARTIAL});

    methods.push_back({"getVictoryMe", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getVictoryMe());
    }, CompatStatus::PARTIAL});

    methods.push_back({"getDefeatMe", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getDefeatMe());
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"getPhase", [](const std::vector<Value>&) -> Value {
        return Value::Int(static_cast<int32_t>(BattleManager::instance().getPhase()));
    }, CompatStatus::FULL});
    
    methods.push_back({"getTurnCount", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().getTurnCount());
    }, CompatStatus::FULL});
    
    methods.push_back({"processEscape", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().processEscape() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"processAction", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().processAction();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"queueAction", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t actionType = 0;
        int32_t targetIndex = -1;
        int32_t skillId = 0;
        int32_t itemId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) actionType = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        if (args.size() > 3 && std::holds_alternative<int64_t>(args[3].v)) targetIndex = static_cast<int32_t>(std::get<int64_t>(args[3].v));
        if (args.size() > 4 && std::holds_alternative<int64_t>(args[4].v)) skillId = static_cast<int32_t>(std::get<int64_t>(args[4].v));
        if (args.size() > 5 && std::holds_alternative<int64_t>(args[5].v)) itemId = static_cast<int32_t>(std::get<int64_t>(args[5].v));
        BattleManager::instance().queueActionByIndices(subjectIndex, static_cast<BattleSubjectType>(subjectType), static_cast<BattleActionType>(actionType), targetIndex, skillId, itemId);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getNextAction", [](const std::vector<Value>&) -> Value {
        BattleAction* action = BattleManager::instance().getNextAction();
        if (!action || !action->subject) return Value::Nil();
        Object obj;
        obj["subjectIndex"] = Value::Int(action->subject->index);
        obj["subjectType"] = Value::Int(static_cast<int32_t>(action->subject->type));
        obj["type"] = Value::Int(static_cast<int32_t>(action->type));
        obj["targetIndex"] = Value::Int(action->targetIndex);
        obj["skillId"] = Value::Int(action->skillId);
        obj["itemId"] = Value::Int(action->itemId);
        return Value::Obj(std::move(obj));
    }, CompatStatus::FULL});
    
    methods.push_back({"clearActions", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().clearActions();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"checkTurnCondition", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Int(0);
        int32_t turn = 0;
        int32_t span = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) turn = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) span = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        return Value::Int(BattleManager::instance().checkTurnCondition(turn, span) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"forceAction", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 4) return Value::Nil();
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t actionType = 0;
        int32_t targetIndex = -1;
        int32_t skillId = 0;
        int32_t itemId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) actionType = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        if (std::holds_alternative<int64_t>(args[3].v)) targetIndex = static_cast<int32_t>(std::get<int64_t>(args[3].v));
        if (args.size() > 4 && std::holds_alternative<int64_t>(args[4].v)) skillId = static_cast<int32_t>(std::get<int64_t>(args[4].v));
        if (args.size() > 5 && std::holds_alternative<int64_t>(args[5].v)) itemId = static_cast<int32_t>(std::get<int64_t>(args[5].v));
        BattleManager::instance().forceAction(subjectIndex, static_cast<BattleSubjectType>(subjectType), static_cast<BattleActionType>(actionType), targetIndex, skillId, itemId);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"addState", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().addState(subject, stateId, 3) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"removeState", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().removeState(subject, stateId) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"isStateActive", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().isStateActive(subject, stateId) ? 1 : 0);
    }, CompatStatus::FULL});
    
    ctx.registerObject("BattleManager", methods);
}

} // namespace compat
} // namespace urpg
