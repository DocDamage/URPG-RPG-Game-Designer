#pragma once

#include "engine/core/math/fixed32.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace urpg {

struct ActorStats {
    int32_t level = 1;
    int32_t hp = 0;
    int32_t maxHp = 0;
    int32_t mp = 0;
    int32_t maxMp = 0;
    Fixed32 atk = Fixed32::FromInt(0);
    Fixed32 def = Fixed32::FromInt(0);
    int32_t agility = 0;
    int32_t luck = 0;

    // Wave 3 Additions: Element, Weakness, and Resistances
    std::string element = "None";
    std::unordered_map<std::string, float> weaknesses;
    std::unordered_map<std::string, float> resistances;
};

struct DamageResult {
    int32_t damage = 0;
    bool critical = false;
    bool weak = false; // Wave 3: True if target was weak to element
};

class CombatCalc {
  public:
    DamageResult PhysicalDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const;

    // Wave 3: Advanced Damage calculation including elemental bonuses
    DamageResult CalculateDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const;
    DamageResult CalculateDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed,
                                 const std::string& skill_id) const;
};

} // namespace urpg
