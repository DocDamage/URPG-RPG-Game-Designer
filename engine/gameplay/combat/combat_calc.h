#pragma once

#include "engine/core/math/fixed32.h"

#include <cstdint>

namespace urpg {

struct ActorStats {
    int32_t level = 1;
    Fixed32 atk = Fixed32::FromInt(0);
    Fixed32 def = Fixed32::FromInt(0);
};

struct DamageResult {
    int32_t damage = 0;
    bool critical = false;
};

class CombatCalc {
public:
    DamageResult PhysicalDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const;
};

} // namespace urpg
