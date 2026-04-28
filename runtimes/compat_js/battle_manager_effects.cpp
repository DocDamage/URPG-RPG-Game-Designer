#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_support.h"
#include "runtimes/compat_js/data_manager.h"

#include <algorithm>
#include <vector>

namespace urpg {
namespace compat {
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
    const CompatFormulaAmount formulaAmount =
        resolveCompatFormulaAmount(skill->damage, user, target, skill, nullptr);
    const int32_t dmgType = skill->damage.type;
    if (dmgType == 1 || dmgType == 2) {
        int32_t amount = formulaAmount.usedFormula
            ? formulaAmount.amount
            : resolveAttackDamage(user, target, formulaAmount.amount);
        applyDamage(target, amount, dmgType == 1);
    } else if (dmgType == 3 || dmgType == 4) {
        applyHeal(target, formulaAmount.amount, dmgType == 3);
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
    const CompatFormulaAmount formulaAmount =
        resolveCompatFormulaAmount(item->damage, user, target, nullptr, item);
    const int32_t dmgType = item->damage.type;
    if (dmgType == 1 || dmgType == 2) {
        int32_t amount = formulaAmount.usedFormula
            ? formulaAmount.amount
            : resolveAttackDamage(user, target, formulaAmount.amount);
        applyDamage(target, amount, dmgType == 1);
    } else if (dmgType == 3 || dmgType == 4) {
        applyHeal(target, formulaAmount.amount, dmgType == 3);
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

} // namespace compat
} // namespace urpg
