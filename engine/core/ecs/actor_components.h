#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace urpg {

/**
 * @brief Core stats shared by all combat participants.
 */
struct ActorStats {
    int32_t hp = 0;
    int32_t maxHp = 0;
    int32_t mp = 0;
    int32_t maxMp = 0;
    
    int32_t attack = 0;
    int32_t defense = 0;
    int32_t agility = 0;
    int32_t luck = 0;
};

/**
 * @brief Component for identifying an entity as an Actor (PC or NPC).
 */
struct ActorComponent {
    std::string name;
    std::string className;
    uint32_t level = 1;
    ActorStats stats;
    bool isEnemy = false;
};

/**
 * @brief Component for position on the map (tiles).
 */
struct TransformComponent {
    int32_t x = 0;
    int32_t y = 0;
};

/**
 * @brief Component for a sprite or visual representation.
 */
struct VisualComponent {
    std::string assetPath;
    int32_t frameIndex = 0;
    bool visible = true;
};

} // namespace urpg
