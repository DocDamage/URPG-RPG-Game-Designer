#include "runtimes/compat_js/battle_manager_support.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <utility>

namespace urpg {
namespace compat {
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

int32_t resolveBattleAnimationDurationFrames(int32_t animationId) {
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

int32_t valueToInt(const Value& value, int32_t fallback) {
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

bool valueToBool(const Value& value, bool fallback) {
    if (const auto* asBool = std::get_if<bool>(&value.v)) {
        return *asBool;
    }
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        return *asInt != 0;
    }
    return fallback;
}

std::string valueToString(const Value& value, const std::string& fallback) {
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

bool isBlankFormula(const std::string& formula) {
    return std::all_of(formula.begin(),
                       formula.end(),
                       [](unsigned char ch) { return std::isspace(ch) != 0; });
}

urpg::scene::BattleParticipant makeFormulaParticipant(const BattleSubject* subject) {
    urpg::scene::BattleParticipant participant;
    if (!subject) {
        participant.id = "0";
        participant.hp = 0;
        participant.maxHp = 0;
        participant.mp = 0;
        participant.maxMp = 0;
        participant.isEnemy = false;
        return participant;
    }

    participant.id = std::to_string(subject->id);
    participant.name = subject->type == BattleSubjectType::ACTOR ? "compat_actor" : "compat_enemy";
    participant.hp = subject->hp;
    participant.maxHp = subject->mhp;
    participant.mp = subject->mp;
    participant.maxMp = subject->mmp;
    participant.isEnemy = subject->type == BattleSubjectType::ENEMY;
    return participant;
}

QuickJSContext::MethodDef makeMethodDef(std::string name,
                                        QuickJSContext::HostFunction fn,
                                        CompatStatus status,
                                        std::string deviationNote) {
    QuickJSContext::MethodDef method;
    method.name = std::move(name);
    method.fn = std::move(fn);
    method.status = status;
    method.deviationNote = std::move(deviationNote);
    return method;
}

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

bool isBattleRunning(BattlePhase phase) {
    return phase != BattlePhase::NONE && phase != BattlePhase::END;
}

} // namespace compat
} // namespace urpg
