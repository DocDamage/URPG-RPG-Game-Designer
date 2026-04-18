#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "editor/ability/ability_inspector_panel.h"
#include <iostream>
#include <cassert>

using namespace urpg;
using namespace urpg::editor;

int main() {
    std::cout << "Testing Ability Inspector UI Projection...\n";

    AbilitySystemComponent asc;
    AbilityInspectorPanel panel;

    // 1. Setup ability
    GameplayAbility firebol;
    firebol.name = "Firebolt";
    firebol.cooldown = 5.0f;
    firebol.requiredTags.add("State.Mana.High");

    // 2. Add tag to ASC
    asc.addTag("State.Mana.High");
    asc.addTag("Buff.Haste");

    // 3. Activate ability (starts cooldown)
    bool success = firebol.activate(asc);
    assert(success);
    assert(asc.isOnCooldown("Firebolt"));

    // 4. Update panel
    panel.update(asc);

    // 5. Render
    std::cout << "\n[Initial State - After Activation]\n";
    panel.render();

    // 6. Simulate time pass
    asc.update(2.5f);
    panel.update(asc);
    std::cout << "\n[Mid-Cooldown State]\n";
    panel.render();

    // 7. Expire cooldown
    asc.update(3.0f);
    panel.update(asc);
    std::cout << "\n[Cooldown Expired State]\n";
    panel.render();

    std::cout << "\nAbility Inspector test completed successfully.\n";
    return 0;
}
