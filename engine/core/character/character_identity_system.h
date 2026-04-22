#pragma once

#include "engine/core/character/character_identity.h"
#include "engine/core/ecs/world.h"
#include "engine/core/math/fixed32.h"

namespace urpg {

/**
 * @brief System that synchronizes CharacterIdentityComponent data into
 * ActorComponent and VisualComponent.
 *
 * Propagation rules:
 *   - identity.name -> actor.name
 *   - identity.classId -> actor.className
 *   - identity.bodySpriteId -> visual.assetPath
 *   - identity.appearanceTokens -> visual.appearanceTags (for layered rendering)
 */
class CharacterIdentitySystem {
public:
    void update(World& world);
};

/**
 * @brief Deterministic helper for spawning an actor from a CharacterIdentity.
 */
class CharacterSpawner {
public:
    struct Request {
        character::CharacterIdentity identity;
        Fixed32 x = Fixed32::FromInt(0);
        Fixed32 y = Fixed32::FromInt(0);
        Fixed32 z = Fixed32::FromInt(0);
        bool isEnemy = false;
    };

    struct Result {
        EntityID entity = 0;
        bool success = false;
    };

    /**
     * @brief Spawn an actor entity with CharacterIdentityComponent attached.
     */
    static Result spawn(World& world, class ActorManager& actorManager, const Request& request);
};

} // namespace urpg
