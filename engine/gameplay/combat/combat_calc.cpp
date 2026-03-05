#include "engine/gameplay/combat/combat_calc.h"

#include <algorithm>

namespace urpg {

DamageResult CombatCalc::PhysicalDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const {
    const int32_t base = std::max<int32_t>(0, (a.atk - d.def).raw >> 16);

    int32_t variance_percent = 0;
    if (variance_seed != 0) {
        variance_percent = static_cast<int32_t>(variance_seed % 11) - 5;
    }

    const int32_t adjusted = base + (base * variance_percent) / 100;

    DamageResult result;
    result.damage = std::max(0, adjusted);
    result.critical = false;
    return result;
}

} // namespace urpg
