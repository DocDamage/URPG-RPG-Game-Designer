#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component to hold experience points and level-up logic.
 */
struct ExperienceComponent {
    uint32_t currentExp = 0;
    uint32_t nextLevelExp = 100;
    uint32_t level = 1;
    float growthRate = 1.15f; // Exponential growth for leveling
};

/**
 * @brief Component for earning rewards upon defeating an entity.
 */
struct LootComponent {
    uint32_t expReward = 10;
    uint32_t goldReward = 5;
    std::string lootTableName;
};

} // namespace urpg
