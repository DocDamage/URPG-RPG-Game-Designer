#pragma once

#include "engine/core/character/character_identity.h"

namespace urpg {

/**
 * @brief ECS component that attaches a CharacterIdentity to an entity.
 *
 * When present alongside ActorComponent and VisualComponent, the
 * CharacterIdentitySystem will propagate identity fields (name, class,
 * appearance tokens) into those runtime components.
 */
struct CharacterIdentityComponent {
    character::CharacterIdentity identity;
    bool dirty = true;
};

} // namespace urpg
