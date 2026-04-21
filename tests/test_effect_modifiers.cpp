#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_effect.h"

using namespace urpg;
using namespace urpg::ability;

TEST_CASE("Effect Modifiers and Attribute Selection", "[ability]") {
    AbilitySystemComponent asc;
    float baseAttack = 100.0f;

    SECTION("Initial state") {
        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 100.0f);
    }

    SECTION("Add modifier") {
        GameplayEffect strengthBuff;
        strengthBuff.name = "Strength Buff";
        strengthBuff.duration = 10.0f;
        strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
        asc.applyEffect(strengthBuff);

        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 120.0f);
    }

    SECTION("Multiply modifier") {
        GameplayEffect strengthBuff;
        strengthBuff.name = "Strength Buff";
        strengthBuff.duration = 10.0f;
        strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
        asc.applyEffect(strengthBuff);

        GameplayEffect haste;
        haste.name = "Haste";
        haste.duration = -1.0f; // Permanent
        haste.modifiers.push_back({"Attack", ModifierOp::Multiply, 1.5f});
        asc.applyEffect(haste);

        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 180.0f); // (100 + 20) * 1.5
    }

    SECTION("Tag-conditional modifier") {
        GameplayEffect strengthBuff;
        strengthBuff.name = "Strength Buff";
        strengthBuff.duration = 10.0f;
        strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
        asc.applyEffect(strengthBuff);

        GameplayEffect haste;
        haste.name = "Haste";
        haste.duration = -1.0f;
        haste.modifiers.push_back({"Attack", ModifierOp::Multiply, 1.5f});
        asc.applyEffect(haste);

        GameplayEffect situational;
        situational.name = "Situational Power";
        situational.duration = -1.0f;
        situational.modifiers.push_back({"Attack", ModifierOp::Add, 50.0f, "State.Berserk"});
        asc.applyEffect(situational);

        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 180.0f);

        asc.addTag(GameplayTag("State.Berserk"));
        val = asc.getAttribute("Attack", baseAttack);
        // (100 + 20 + 50) * 1.5 = 255.0
        REQUIRE(val == 255.0f);
    }

    SECTION("Override modifier") {
        GameplayEffect strengthBuff;
        strengthBuff.name = "Strength Buff";
        strengthBuff.duration = 10.0f;
        strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
        asc.applyEffect(strengthBuff);

        GameplayEffect haste;
        haste.name = "Haste";
        haste.duration = -1.0f;
        haste.modifiers.push_back({"Attack", ModifierOp::Multiply, 1.5f});
        asc.applyEffect(haste);

        GameplayEffect situational;
        situational.name = "Situational Power";
        situational.duration = -1.0f;
        situational.modifiers.push_back({"Attack", ModifierOp::Add, 50.0f, "State.Berserk"});
        asc.applyEffect(situational);

        asc.addTag(GameplayTag("State.Berserk"));

        GameplayEffect stun;
        stun.name = "Stun";
        stun.duration = 2.0f;
        stun.modifiers.push_back({"Attack", ModifierOp::Override, 0.0f});
        asc.applyEffect(stun);

        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 0.0f);
    }

    SECTION("Update and cleanup") {
        GameplayEffect strengthBuff;
        strengthBuff.name = "Strength Buff";
        strengthBuff.duration = 10.0f;
        strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
        asc.applyEffect(strengthBuff);

        GameplayEffect haste;
        haste.name = "Haste";
        haste.duration = -1.0f;
        haste.modifiers.push_back({"Attack", ModifierOp::Multiply, 1.5f});
        asc.applyEffect(haste);

        GameplayEffect situational;
        situational.name = "Situational Power";
        situational.duration = -1.0f;
        situational.modifiers.push_back({"Attack", ModifierOp::Add, 50.0f, "State.Berserk"});
        asc.applyEffect(situational);

        asc.addTag(GameplayTag("State.Berserk"));

        GameplayEffect stun;
        stun.name = "Stun";
        stun.duration = 2.0f;
        stun.modifiers.push_back({"Attack", ModifierOp::Override, 0.0f});
        asc.applyEffect(stun);

        // Expire stun and buff
        asc.update(3.0f);
        // Note: strengthBuff duration was 10, so it remains. Stun was 2.0, so it's gone.
        float val = asc.getAttribute("Attack", baseAttack);
        REQUIRE(val == 255.0f);

        asc.update(8.0f);
        val = asc.getAttribute("Attack", baseAttack);
        // Remaining: Haste (*1.5) and Berserk (+50)
        // (100 + 50) * 1.5 = 225.0
        REQUIRE(val == 225.0f);
    }
}
