#pragma once

#include "engine/runtimes/bridge/value.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace urpg {
namespace compat {

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

enum class BattleActionType : uint8_t {
    ATTACK = 0,
    GUARD = 1,
    SKILL = 2,
    ITEM = 3,
    ESCAPE = 4,
    WAIT = 5
};

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

struct BattleSubject {
    BattleSubjectType type = BattleSubjectType::ACTOR;
    int32_t index = 0;
    int32_t id = 0;

    int32_t hp = 0;
    int32_t mp = 0;
    int32_t tp = 0;
    int32_t mhp = 0;
    int32_t mmp = 0;

    bool hidden = false;
    bool immortal = false;
    bool acted = false;
    int32_t actionSpeed = 0;

    BattleActionType pendingAction = BattleActionType::WAIT;
    int32_t targetIndex = -1;
    int32_t skillId = 0;
    int32_t itemId = 0;
    std::vector<BattleStateEffect> states;
    std::vector<BattleModifierEffect> modifiers;
};

struct BattleAction {
    BattleSubject* subject = nullptr;
    BattleActionType type = BattleActionType::ATTACK;
    int32_t targetIndex = -1;
    int32_t skillId = 0;
    int32_t itemId = 0;
    int32_t animationId = 0;
    bool forced = false;
};

struct BattleAudioCue {
    std::string name;
    double volume = 90.0;
    double pitch = 100.0;
};

struct BattleAnimationPlayback {
    int32_t animationId = 0;
    BattleSubjectType targetType = BattleSubjectType::ACTOR;
    int32_t targetIndex = 0;
    int32_t targetId = 0;
    int32_t framesRemaining = 0;
    bool subjectAnimation = false;
};

enum class BattleResult : uint8_t {
    NONE = 0,
    WIN = 1,
    ESCAPE = 2,
    DEFEAT = 3,
    ABORT = 4
};

enum class BattleHookPoint : uint8_t {
    ON_SETUP,
    ON_START,
    ON_TURN_START,
    ON_TURN_END,
    ON_ACTION_START,
    ON_ACTION_END,
    ON_DAMAGE,
    ON_HEAL,
    ON_STATE_ADDED,
    ON_STATE_REMOVED,
    ON_ACTOR_DEATH,
    ON_ENEMY_DEATH,
    ON_VICTORY,
    ON_DEFEAT,
    ON_ESCAPE,
    ON_ABORT
};

using BattleHookFn = std::function<Value(const std::vector<Value>& args)>;

} // namespace compat
} // namespace urpg
