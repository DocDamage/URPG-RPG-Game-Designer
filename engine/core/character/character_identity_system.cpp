#include "engine/core/character/character_identity_system.h"
#include "engine/core/character/character_identity_component.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"

namespace urpg {

void CharacterIdentitySystem::update(World& world) {
    world.ForEachWith<CharacterIdentityComponent, ActorComponent, VisualComponent>(
        [&](EntityID /*id*/, CharacterIdentityComponent& identity, ActorComponent& actor, VisualComponent& visual) {
            if (!identity.dirty) {
                return;
            }

            const auto& cid = identity.identity;

            if (!cid.getName().empty()) {
                actor.name = cid.getName();
            }
            if (!cid.getClassId().empty()) {
                actor.className = cid.getClassId();
            }
            if (!cid.getBodySpriteId().empty()) {
                visual.assetPath = cid.getBodySpriteId();
            }

            identity.dirty = false;
        });
}

CharacterSpawner::Result CharacterSpawner::spawn(World& world, ActorManager& actorManager, const Request& request) {
    Result result;

    EntityID id = actorManager.CreateActor(request.identity.getDisplayName(), request.isEnemy);
    if (id == 0) {
        return result;
    }

    actorManager.SetActorPosition(id, request.x, request.y, request.z);

    CharacterIdentityComponent identityComp;
    identityComp.identity = request.identity;
    identityComp.dirty = true;
    world.AddComponent(id, identityComp);

    // Run one sync immediately so the actor is fully configured
    CharacterIdentitySystem system;
    system.update(world);

    result.entity = id;
    result.success = true;
    return result;
}

} // namespace urpg
