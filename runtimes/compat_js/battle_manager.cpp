// BattleManager - MZ Battle Pipeline Hooks - Implementation
// Phase 2 - Compat Layer

#include "battle_manager.h"
#include "runtimes/compat_js/battle_manager_support.h"
#include "runtimes/compat_js/battle_manager_internal_state.h"
#include "audio_manager.h"
#include "data_manager.h"
#include "engine/core/scene/combat_formula.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <random>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> BattleManager::methodStatus_;
std::unordered_map<std::string, std::string> BattleManager::methodDeviations_;

namespace {

} // namespace

// Internal implementation
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
                  "Transition type is still metadata-only, but battle audio cues now drive the compat AudioManager harness during battle lifecycle.");
        setStatus("setBattleBackground", CompatStatus::PARTIAL,
                  "Battle background name is still metadata-only; no live BattleScene background routing exists yet.");
        setStatus("setBattleBgm", CompatStatus::PARTIAL,
                  "Battle BGM now drives the compat AudioManager harness during battle lifecycle, but still does not reach a live scene/audio backend.");
        setStatus("setVictoryMe", CompatStatus::PARTIAL,
                  "Victory ME now drives the compat AudioManager harness on battle end, but still does not reach a live scene/audio backend.");
        setStatus("setDefeatMe", CompatStatus::PARTIAL,
                  "Defeat ME now drives the compat AudioManager harness on battle end, but still does not reach a live scene/audio backend.");
        setStatus("changeBattleBackground", CompatStatus::PARTIAL,
                  "Battle background name is still metadata-only; no live BattleScene background routing exists yet.");
        setStatus("changeBattleBgm", CompatStatus::PARTIAL,
                  "Battle BGM now drives the compat AudioManager harness during battle lifecycle, but still does not reach a live scene/audio backend.");
        setStatus("changeVictoryMe", CompatStatus::PARTIAL,
                  "Victory ME now drives the compat AudioManager harness on battle end, but still does not reach a live scene/audio backend.");
        setStatus("changeDefeatMe", CompatStatus::PARTIAL,
                  "Defeat ME now drives the compat AudioManager harness on battle end, but still does not reach a live scene/audio backend.");
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
        setStatus("applySkill", CompatStatus::PARTIAL, kApplySkillDeviationBase);
        setStatus("applyItem", CompatStatus::PARTIAL, kApplyItemDeviationBase);
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
        setStatus("startBattleEvent", CompatStatus::PARTIAL, kBattleEventDeviationBase);
        setStatus("updateBattleEvents", CompatStatus::PARTIAL, kBattleEventDeviationBase);
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

} // namespace compat
} // namespace urpg
