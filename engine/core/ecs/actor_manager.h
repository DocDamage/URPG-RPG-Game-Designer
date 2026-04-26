#pragma once

#include "actor_components.h"
#include "engine/core/character/character_identity.h"
#include "engine/core/character/character_identity_component.h"
#include "world.h"
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
        world_.AddComponent(id, VelocityComponent{});

        return id;
    }

    /**
     * @brief Set location of an actor.
     */
    void SetActorPosition(EntityID id, Fixed32 x, Fixed32 y, Fixed32 z = Fixed32::FromInt(0)) {
        if (auto* transform = world_.GetComponent<TransformComponent>(id)) {
            transform->position = {x, y, z};
        }
    }

    /**
     * @brief Set velocity of an actor.
     */
    void SetActorVelocity(EntityID id, Fixed32 vx, Fixed32 vy, Fixed32 vz = Fixed32::FromInt(0)) {
        if (auto* velocity = world_.GetComponent<VelocityComponent>(id)) {
            velocity->linear = {vx, vy, vz};
        }
    }

    /**
     * @brief Get Actor component data.
     */
    ActorComponent* GetActor(EntityID id) { return world_.GetComponent<ActorComponent>(id); }

    /**
     * @brief Check if actor is an enemy.
     */
    bool IsEnemy(EntityID id) {
        if (auto* actor = world_.GetComponent<ActorComponent>(id)) {
            return actor->isEnemy;
        }
        return false;
    }

    /**
     * @brief Create an actor from a CharacterIdentity, attaching identity component.
     * @param identity The character identity to spawn.
     * @param isEnemy True if the actor is an enemy, false for PC.
     * @return The EntityID of the new actor.
     */
    EntityID CreateActorFromIdentity(const character::CharacterIdentity& identity, bool isEnemy = false) {
        EntityID id = CreateActor(identity.getDisplayName(), isEnemy);

        CharacterIdentityComponent comp;
        comp.identity = identity;
        comp.dirty = true;
        world_.AddComponent(id, comp);

        if (!identity.getBodySpriteId().empty()) {
            if (auto* visual = world_.GetComponent<VisualComponent>(id)) {
                visual->assetPath = identity.getBodySpriteId();
            }
        }

        return id;
    }

  private:
    World& world_;
};

} // namespace urpg
