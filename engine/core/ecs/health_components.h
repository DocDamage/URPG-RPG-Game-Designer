#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component that provides health and basic life-tracking.
 */
struct HealthComponent {
    int32_t current = 100;
    int32_t max = 100;
    bool isAlive = true;
    bool destroyOnDeath = false;
};

/**
 * @brief Component for an entity that provides a "Damage" effect on contact.
 */
struct DamageOnContactComponent {
    int32_t damage = 10;
    Fixed32 cooldown = Fixed32::FromInt(1);
    Fixed32 lastTriggerTime = Fixed32::FromInt(-1);
};

/**
 * @brief Component to mark an entity as "Invulnerable" for a duration after hit.
 */
struct InvulnerabilityComponent {
    Fixed32 duration = Fixed32::FromInt(1);
    Fixed32 timer = Fixed32::FromInt(0);
};

} // namespace urpg
