// INCUBATING TEST: This file contains a standalone main() and is not yet
// integrated into the Catch2 test suite (urpg_tests). Do not register it in
// CMakeLists.txt until it is converted to Catch2 TEST_CASE macros.
//
#include "engine/core/ability/ability_state_machine.h"
#include "engine/core/ability/ability_system_component.h"
#include <iostream>
#include <cassert>

using namespace urpg;

int main() {
    std::cout << "Testing Ability State Machine - Multi-Wave Logic\n";

    AbilitySystemComponent asc;
    AbilityStateMachine chain("FireballChain");

    bool windupEntered = false;
    bool impactEntered = false;
    bool recoveryEntered = false;

    // 1. Windup State (Wait 1.0s)
    AbilityState windup;
    windup.name = "Windup";
    windup.inherentTags.add("State.Immune.Stagger");
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
    assert(windupEntered);
    assert(asc.hasTag("State.Immune.Stagger"));

    // Tick 0.5s
    chain.update(asc, 0.5f);
    assert(!impactEntered);

    // Finish Windup, Start Impact/Recovery
    chain.update(asc, 0.6f);
    assert(impactEntered);
    assert(recoveryEntered);
    assert(!asc.hasTag("State.Immune.Stagger")); // Windup tag should be gone

    // Finish Recovery
    chain.update(asc, 1.0f);
    assert(chain.getStatus() == AbilityStateStatus::Finished);

    std::cout << "Multi-Wave Ability State Machine test completed successfully.\n";
    return 0;
}
