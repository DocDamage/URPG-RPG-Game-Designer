#pragma once

#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/data_manager.h"
#include "engine/core/scene/combat_formula.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace urpg {
namespace compat {

constexpr int32_t kAttackParamId = 2;
constexpr int32_t kDefenseParamId = 3;
constexpr int32_t kRecoveryParamId = 4;
constexpr int32_t kAgilityParamId = 6;
constexpr int32_t kVictorySwitchId = 101;
constexpr int32_t kDefeatSwitchId = 102;
constexpr int32_t kEscapeSwitchId = 103;

struct CompatFormulaAmount {
    int32_t amount = 0;
    bool usedFormula = false;
    std::string fallbackReason;
};

Value battleAudioCueToValue(const BattleAudioCue& cue);
int32_t resolveBattleAnimationDurationFrames(int32_t animationId);
const Object* asObject(const Value& value);
const Array* asArray(const Value& value);
int32_t valueToInt(const Value& value, int32_t fallback = 0);
bool valueToBool(const Value& value, bool fallback = false);
std::string valueToString(const Value& value, const std::string& fallback = "");
BattleStateEffect* findStateEffect(BattleSubject* subject, int32_t stateId);
BattleModifierEffect* findModifierEffect(BattleSubject* subject, int32_t paramId);
int32_t clampModifierStage(int32_t stage);
double computeBaseEscapeRatio(const std::vector<BattleSubject>& actors, const std::vector<BattleSubject>& enemies);
uint32_t mixEscapeSeed(int32_t troopId);
double nextEscapeRoll(uint32_t& state);
bool isBlankFormula(const std::string& formula);
urpg::scene::BattleParticipant makeFormulaParticipant(const BattleSubject* subject);
QuickJSContext::MethodDef makeMethodDef(std::string name, QuickJSContext::HostFunction fn, CompatStatus status, std::string deviationNote = {});
int32_t getModifierStage(const BattleSubject* subject, int32_t paramId);
double modifierMultiplier(int32_t stage);
int32_t scaleMagnitude(int32_t amount, int32_t stage);
int32_t resolveAttackDamage(const BattleSubject* subject, const BattleSubject* target, int32_t baseDamage);
double targetPriorityScore(const BattleSubject* target);
BattleSubject* resolveActionTarget(BattleManager* manager, const BattleAction& action);
void syncActorSubjectToDataManager(const BattleSubject* subject);
bool isBattleRunning(BattlePhase phase);

template <typename DamageData>
CompatFormulaAmount resolveCompatFormulaAmount(const DamageData& damage,
                                               BattleSubject* user,
                                               BattleSubject* target,
                                               const SkillData* skill,
                                               const ItemData* item) {
    CompatFormulaAmount result;
    result.amount = std::max(0, damage.power);

    if (isBlankFormula(damage.formula)) {
        return result;
    }

    urpg::scene::BattleParticipant userParticipant = makeFormulaParticipant(user);
    urpg::scene::BattleParticipant targetParticipant = makeFormulaParticipant(target);
    urpg::combat::CombatFormula::Context ctx{
        user ? &userParticipant : nullptr,
        target ? &targetParticipant : nullptr,
        skill,
        item
    };

    const auto evaluation = urpg::combat::CombatFormula::evaluateFormula(damage.formula, ctx);
    if (evaluation.usedFallback) {
        result.fallbackReason = evaluation.reason;
        return result;
    }

    result.amount = std::max(0, evaluation.value);
    result.usedFormula = true;
    return result;
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

} // namespace compat
} // namespace urpg
