#pragma once

#include "runtimes/compat_js/battle_manager.h"

#include <cstdint>
#include <random>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {
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


} // namespace compat
} // namespace urpg
