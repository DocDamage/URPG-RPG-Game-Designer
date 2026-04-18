#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_effect.h"
#include <iostream>
#include <cassert>

using namespace urpg;
using namespace urpg::ability;

int main() {
    std::cout << "Testing Effect Modifiers and Attribute Selection...\n";

    AbilitySystemComponent asc;
    float baseAttack = 100.0f;

    // 1. Initial State
    float val = asc.getAttribute("Attack", baseAttack);
    std::cout << "Initial Attack: " << val << "\n";
    assert(val == 100.0f);

    // 2. Add Modifier
    GameplayEffect strengthBuff;
    strengthBuff.name = "Strength Buff";
    strengthBuff.duration = 10.0f;
    strengthBuff.modifiers.push_back({"Attack", ModifierOp::Add, 20.0f});
    asc.applyEffect(strengthBuff);

    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "After +20 Buff: " << val << "\n";
    assert(val == 120.0f);

    // 3. Multiply Modifier
    GameplayEffect haste;
    haste.name = "Haste";
    haste.duration = -1.0f; // Permanent
    haste.modifiers.push_back({"Attack", ModifierOp::Multiply, 1.5f});
    asc.applyEffect(haste);

    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "After (*1.5): " << val << " (Expected 180.0)\n";
    assert(val == 180.0f); // (100 + 20) * 1.5

    // 4. Tag-Conditional Modifier
    GameplayEffect situational;
    situational.name = "Situational Power";
    situational.duration = -1.0f;
    situational.modifiers.push_back({"Attack", ModifierOp::Add, 50.0f, "State.Berserk"});
    asc.applyEffect(situational);

    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "Without tag: " << val << " (Expected 180.0)\n";
    assert(val == 180.0f);

    asc.addTag("State.Berserk");
    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "With Berserk tag: " << val << " (Expected 255.0)\n";
    // (100 + 20 + 50) * 1.5 = 255.0
    assert(val == 255.0f);

    // 5. Override Modifier
    GameplayEffect stun;
    stun.name = "Stun";
    stun.duration = 2.0f;
    stun.modifiers.push_back({"Attack", ModifierOp::Override, 0.0f});
    asc.applyEffect(stun);

    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "After Overriding to 0: " << val << "\n";
    assert(val == 0.0f);

    // 6. Update and Cleanup
    std::cout << "Updating 3.0s (Stun and Buff should expire)...\n";
    asc.update(3.0f);
    // Note: strengthBuff duration was 10, so it remains. Stun was 2.0, so it's gone.
    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "After stun expiration: " << val << " (Expected 255.0)\n";
    assert(val == 255.0f);

    std::cout << "Updating 8.0s (Buff should expire now)...\n";
    asc.update(8.0f);
    val = asc.getAttribute("Attack", baseAttack);
    std::cout << "After buff expiration: " << val << " (Expected 225.0)\n";
    // Remaining: Haste (*1.5) and Berserk (+50)
    // (100 + 50) * 1.5 = 225.0
    assert(val == 225.0f);

    std::cout << "Effect Modifier tests completed successfully.\n";
    return 0;
}
