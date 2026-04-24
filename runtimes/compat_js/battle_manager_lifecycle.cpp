#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_internal_state.h"
#include "runtimes/compat_js/battle_manager_support.h"
#include "runtimes/compat_js/audio_manager.h"
#include "runtimes/compat_js/data_manager.h"

#include <algorithm>

namespace urpg {
namespace compat {
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

    if (isBattleRunning(phase_) && !battleBgm_.name.empty()) {
        AudioManager::instance().playBgm(battleBgm_.name, battleBgm_.volume, battleBgm_.pitch);
    }
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
    AudioManager& audio = AudioManager::instance();
    audio.saveBgmSettings();
    if (!battleBgm_.name.empty()) {
        audio.playBgm(battleBgm_.name, battleBgm_.volume, battleBgm_.pitch);
    }

    phase_ = BattlePhase::START;
    triggerHook(HookPoint::ON_START, {});

    // Enter input phase
    phase_ = BattlePhase::INPUT;
}

void BattleManager::endBattle(BattleResult result) {
    result_ = result;
    phase_ = BattlePhase::END;
    DataManager& dm = DataManager::instance();
    AudioManager& audio = AudioManager::instance();

    switch (result) {
        case BattleResult::WIN:
            dm.setSwitch(kVictorySwitchId, true);
            audio.stopBgm();
            if (!victoryMe_.name.empty()) {
                audio.playMe(victoryMe_.name, victoryMe_.volume, victoryMe_.pitch);
            }
            triggerHook(HookPoint::ON_VICTORY, {});
            break;
        case BattleResult::DEFEAT:
            dm.setSwitch(kDefeatSwitchId, true);
            audio.stopBgm();
            if (!defeatMe_.name.empty()) {
                audio.playMe(defeatMe_.name, defeatMe_.volume, defeatMe_.pitch);
            }
            triggerHook(HookPoint::ON_DEFEAT, {});
            break;
        case BattleResult::ESCAPE:
            dm.setSwitch(kEscapeSwitchId, true);
            audio.stopMe();
            audio.restoreBgmSettings();
            triggerHook(HookPoint::ON_ESCAPE, {});
            break;
        case BattleResult::ABORT:
            audio.stopMe();
            audio.restoreBgmSettings();
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
} // namespace compat
} // namespace urpg
