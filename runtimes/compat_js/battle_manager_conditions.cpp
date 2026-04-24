#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/data_manager.h"

namespace urpg {
namespace compat {
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
    if (!enemy || enemy->mhp <= 0) {
        return false;
    }
    return (enemy->hp * 100 / enemy->mhp) <= percent;
}

bool BattleManager::checkActorHpCondition(int32_t actorIndex, int32_t percent) {
    BattleSubject* actor = getActor(actorIndex);
    if (!actor || actor->mhp <= 0) {
        return false;
    }
    return (actor->hp * 100 / actor->mhp) <= percent;
}

bool BattleManager::checkSwitchCondition(int32_t switchId) {
    return DataManager::instance().getSwitch(switchId);
}

} // namespace compat
} // namespace urpg
