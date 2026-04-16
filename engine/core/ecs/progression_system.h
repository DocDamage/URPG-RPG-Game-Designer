#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/health_components.h"
#include "engine/core/ecs/progression_components.h"
#include <vector>

namespace urpg {

/**
 * @brief System that processes entity deaths and awards XP/Loot to the player.
 */
class ProgressionSystem {
public:
    void update(World& world) {
        // Collect deaths this frame
        std::vector<EntityID> deadEntities;
        world.ForEachWith<HealthComponent>([&](EntityID id, const HealthComponent& health) {
            if (!health.isAlive) {
                deadEntities.push_back(id);
            }
        });

        if (deadEntities.empty()) return;

        // Find the player to award points (assuming one player for now)
        EntityID playerId = 0;
        ExperienceComponent* playerExp = nullptr;
        world.ForEachWith<PlayerControlComponent, ExperienceComponent>([&](EntityID id, PlayerControlComponent&, ExperienceComponent& exp) {
            playerId = id;
            playerExp = &exp;
        });

        for (EntityID deadId : deadEntities) {
            // Process loot and XP rewards
            if (auto* loot = world.GetComponent<LootComponent>(deadId)) {
                if (playerExp) {
                    awardExperience(*playerExp, loot->expReward);
                }
                // Gold and Item rewards would be handled here
            }

            // Check if entity should be cleaned up
            if (auto* health = world.GetComponent<HealthComponent>(deadId)) {
                if (health->destroyOnDeath) {
                    world.DestroyEntity(deadId);
                }
            }
        }
    }

private:
    void awardExperience(ExperienceComponent& exp, uint32_t amount) {
        exp.currentExp += amount;
        while (exp.currentExp >= exp.nextLevelExp) {
            exp.currentExp -= exp.nextLevelExp;
            exp.level++;
            exp.nextLevelExp = static_cast<uint32_t>(static_cast<float>(exp.nextLevelExp) * exp.growthRate);
            // In a full engine, this would trigger a "Level Up" event or stat increase
        }
    }
};

} // namespace urpg
