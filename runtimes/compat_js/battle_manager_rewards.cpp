#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_internal_state.h"
#include "runtimes/compat_js/battle_manager_support.h"
#include "runtimes/compat_js/data_manager.h"

#include <random>
#include <vector>

namespace urpg {
namespace compat {
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
    const int32_t totalExp = calculateExp();
    if (totalExp <= 0) {
        return;
    }

    auto& dm = DataManager::instance();
    for (int32_t i = 0; i < dm.getPartySize(); ++i) {
        const int32_t actorId = dm.getPartyMember(i);
        if (actorId > 0) {
            dm.gainExp(actorId, totalExp);
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
} // namespace compat
} // namespace urpg
