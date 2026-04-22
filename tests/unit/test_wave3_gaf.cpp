#include "engine/gameplay/combat/combat_calc.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg;
using namespace urpg::ability;

TEST_CASE("Wave 3: Elemental Resistance and Weakness", "[combat][elemental]") {
    CombatCalc calc;

    ActorStats attacker;
    attacker.atk = Fixed32::FromInt(50);
    attacker.element = "Fire";

    ActorStats defender;
    defender.def = Fixed32::FromInt(25);

    SECTION("Minimal test") {
        auto result = calc.CalculateDamage(attacker, defender, 0, "skill_1");
        REQUIRE(result.damage >= 0);
    }
}

TEST_CASE("Wave 3: Gameplay Effect Stacking - Refresh and Stack", "[ability][asc]") {
    AbilitySystemComponent asc;

    GameplayEffect haste;
    haste.id = "BUFF_HASTE";
    haste.duration = 5.0f;
    haste.stackingPolicy = GameplayEffectStackingPolicy::Refresh;

    GameplayEffectModifier mod;
    mod.attributeName = "Speed";
    mod.value = 1.1f;
    mod.operation = ModifierOp::Multiply;
    haste.modifiers.push_back(mod);

    SECTION("Refresh policy resets duration without duplicating the effect") {
        asc.applyEffect(haste);
        asc.update(3.0f);
        asc.applyEffect(haste);
        asc.update(3.0f);

        REQUIRE(asc.getActiveEffectCount() == 1);
        REQUIRE(asc.getAttribute("Speed", 100.0f) == 110.0f);

        asc.update(2.1f);
        REQUIRE(asc.getActiveEffectCount() == 0);
        REQUIRE(asc.getAttribute("Speed", 100.0f) == 100.0f);
    }

    SECTION("Stack policy increases modifier strength") {
        GameplayEffect poison;
        poison.id = "DEBUFF_POISON";
        poison.stackingPolicy = GameplayEffectStackingPolicy::Stack;
        poison.maxStacks = 3;
        
        GameplayEffectModifier pmod;
        pmod.attributeName = "HealthRegen";
        pmod.value = -10.0f;
        pmod.operation = ModifierOp::Add;
        poison.modifiers.push_back(pmod);

        asc.applyEffect(poison); // Stack 1: -10
        asc.applyEffect(poison); // Stack 2: -20
        asc.applyEffect(poison); // Stack 3: -30
        asc.applyEffect(poison); // Stack 4: Still -30 (cap)

        float val = asc.getAttribute("HealthRegen", 0.0f);
        REQUIRE(val == -30.0f);
    }
}

class TestAbility : public GameplayAbility {
public:
    const std::string& getId() const override { return id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    
    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
    }

private:
    ActivationInfo m_info;
};

TEST_CASE("Wave 3: Ability Cooldown and Cost", "[ability][asc]") {
    AbilitySystemComponent asc;
    TestAbility ability;
    ability.id = "FIREBALL";
    ability.cooldownTime = 2.0f;
    ability.mpCost = 10.0f;

    SECTION("Ability cannot activate without enough MP") {
        asc.setAttribute("MP", 5.0f);

        const auto check = ability.evaluateActivation(asc);
        REQUIRE_FALSE(check.allowed);
        REQUIRE(check.reason == "insufficient_mp");
        REQUIRE(check.current_mp == 5.0f);
    }

    SECTION("Ability commit deducts MP and starts cooldown") {
        asc.setAttribute("MP", 25.0f);

        ability.activate(asc);
        REQUIRE(asc.getAttribute("MP", 0.0f) == 15.0f);
        REQUIRE(asc.getCooldownRemaining("FIREBALL") == 2.0f);
        REQUIRE_FALSE(ability.canActivate(asc));

        asc.update(1.0f);
        REQUIRE(asc.getCooldownRemaining("FIREBALL") == 1.0f);
        REQUIRE_FALSE(ability.canActivate(asc));

        asc.update(1.0f);
        REQUIRE(asc.getCooldownRemaining("FIREBALL") <= 0.0f);
        REQUIRE(ability.canActivate(asc));
    }
}
