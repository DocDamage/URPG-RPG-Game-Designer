#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component to attach a status effect (e.g., Poison, Burn) to an entity.
 */
struct StatusEffectComponent {
    enum class EffectType : uint8_t {
        Poison,
        Burn,
        Regen,
        Stun
    };

    EffectType type;
    Fixed32 duration = Fixed32::FromInt(5);
    Fixed32 tickInterval = Fixed32::FromInt(1);
    Fixed32 lastTickTime = Fixed32::FromInt(0);
    int32_t power = 5; // Damage or heal per tick
};

} // namespace urpg
