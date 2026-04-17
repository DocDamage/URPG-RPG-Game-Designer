#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/gameplay/inventory_components.h"
#include "engine/core/ecs/actor_components.h"

namespace urpg {

/**
 * @brief Handles logic for dropped items and looting.
 */
struct LootableComponent {
    std::string itemId;
    int amount;
};

class LootSystem {
public:
    void update(World& world) {
        // This would typically check for collisions between player and lootables.
        // For now, let's assume proximity-based looting or event-driven.
    }

    void loot(World& world, EntityID looter, EntityID target) {
        if (!world.HasComponent<InventoryComponent>(looter) || !world.HasComponent<LootableComponent>(target)) {
            return;
        }

        auto* inv = world.GetComponent<InventoryComponent>(looter);
        auto* loot = world.GetComponent<LootableComponent>(target);
        if (!inv || !loot) {
            return;
        }

        if (inv->addItem(loot->itemId, loot->amount)) {
            world.DestroyEntity(target);
        }
    }
};

} // namespace urpg
