#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/ability_state_machine.h"
#include "engine/core/ability/ability_system_component.h"

using namespace urpg;

TEST_CASE("Multi-Wave Ability State Machine", "[ability]") {
    AbilitySystemComponent asc;
    AbilityStateMachine chain("FireballChain");

    bool windupEntered = false;
    bool impactEntered = false;
    bool recoveryEntered = false;

    // 1. Windup State (Wait 1.0s)
    AbilityState windup;
    windup.name = "Windup";
    windup.inherentTags.addTag(urpg::ability::GameplayTag("State.Immune.Stagger"));
    static float timer1 = 0.0f;
    windup.onEnter = [&](AbilitySystemComponent& a) { windupEntered = true; timer1 = 0.0f; };
    windup.onTick = [&](AbilitySystemComponent& a, float dt) {
        timer1 += dt;
        return timer1 >= 1.0f;
    };

    // 2. Impact State (Instant)
    AbilityState impact;
    impact.name = "Impact";
    impact.onEnter = [&](AbilitySystemComponent& a) { impactEntered = true; };
    impact.onTick = [&](AbilitySystemComponent& a, float dt) { return true; }; // Finish instantly

    // 3. Recovery State (Wait 0.5s)
    AbilityState recovery;
    recovery.name = "Recovery";
    static float timer2 = 0.0f;
    recovery.onEnter = [&](AbilitySystemComponent& a) { recoveryEntered = true; timer2 = 0.0f; };
    recovery.onTick = [&](AbilitySystemComponent& a, float dt) {
        timer2 += dt;
        return timer2 >= 0.5f;
    };

    chain.addState(windup);
    chain.addState(impact);
    chain.addState(recovery);

    // Run Test
    chain.start(asc);
    REQUIRE(windupEntered);
    REQUIRE(asc.getTags().hasTag(urpg::ability::GameplayTag("State.Immune.Stagger")));

    // Tick 0.5s
    chain.update(asc, 0.5f);
    REQUIRE_FALSE(impactEntered);

    // Finish Windup, Start Impact
    chain.update(asc, 0.6f);
    REQUIRE(impactEntered);
    REQUIRE_FALSE(recoveryEntered);
    REQUIRE_FALSE(asc.getTags().hasTag(urpg::ability::GameplayTag("State.Immune.Stagger"))); // Windup tag should be gone

    // Finish Impact, Start Recovery
    chain.update(asc, 0.1f);
    REQUIRE(recoveryEntered);

    // Finish Recovery
    chain.update(asc, 1.0f);
    REQUIRE(chain.getStatus() == AbilityStateStatus::Finished);
}
