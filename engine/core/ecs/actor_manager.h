#pragma once

#include "world.h"
#include "actor_components.h"
#include <memory>
#include <string>

namespace urpg {

/**
 * @brief High-level manager for creating and accessing Actors via the ECS.
 */
class ActorManager {
public:
    ActorManager(World& world) : world_(world) {}

    /**
     * @brief Create a new actor entity.
     * @param name The name of the actor.
     * @param isEnemy True if the actor is an enemy, false for PC.
     * @return The EntityID of the new actor.
     */
    EntityID CreateActor(const std::string& name, bool isEnemy = false) {
        EntityID id = world_.CreateEntity();
        
        ActorComponent actor;
        actor.name = name;
        actor.isEnemy = isEnemy;
        world_.AddComponent(id, actor);
        
        world_.AddComponent(id, TransformComponent{});
        world_.AddComponent(id, VisualComponent{});
        
        return id;
    }

    /**
     * @brief Set location of an actor.
     */
    void SetActorPosition(EntityID id, int32_t x, int32_t y) {
        if (auto* transform = world_.GetComponent<TransformComponent>(id)) {
            transform->x = x;
            transform->y = y;
        }
    }

    /**
     * @brief Get Actor component data.
     */
    ActorComponent* GetActor(EntityID id) {
        return world_.GetComponent<ActorComponent>(id);
    }

    /**
     * @brief Check if actor is an enemy.
     */
    bool IsEnemy(EntityID id) {
        if (auto* actor = world_.GetComponent<ActorComponent>(id)) {
            return actor->isEnemy;
        }
        return false;
    }

private:
    World& world_;
};

} // namespace urpg
