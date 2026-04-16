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
    result.weak = false;
    return result;
}

DamageResult CombatCalc::CalculateDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const {
    // Wave 3 standard damage formula: (Atk * 4 - Def * 2) * element_rate
    const int32_t base = std::max<int32_t>(0, (a.atk.raw * 4 - d.def.raw * 2) >> 16);
    
    // Elemental Weakness & Resistance
    float elementRate = 1.0f;

    /*
    // Check resistances (e.g. 0.5 for 50% dmg, 2.0 for 200% dmg)
    auto it_res = d.resistances.find(a.element);
    if (it_res != d.resistances.end()) {
        elementRate *= it_res->second;
    }

    // Check deprecated weaknesses map for backwards compat
    auto it_weak = d.weaknesses.find(a.element);
    if (it_weak != d.weaknesses.end()) {
        elementRate *= it_weak->second;
    }
    */

    // Variance
    int32_t variance_percent = 0;
    if (variance_seed != 0) {
        variance_percent = static_cast<int32_t>(variance_seed % 11) - 5;
    }

    const int32_t adjusted = static_cast<int32_t>(base * elementRate) + (base * variance_percent) / 100;

    DamageResult result;
    result.damage = std::max(0, adjusted);
    result.critical = false;
    result.weak = elementRate > 1.0f;
    return result;
}

} // namespace urpg
