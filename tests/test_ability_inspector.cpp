#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "editor/ability/ability_inspector_panel.h"

using namespace urpg;
using namespace urpg::ability;
using namespace urpg::editor;

class FireboltAbility : public GameplayAbility {
public:
    const std::string& getId() const override {
        static std::string id = "Firebolt";
        return id;
    }

    const ActivationInfo& getActivationInfo() const override {
        static ActivationInfo info;
        info.requiredTags.addTag(GameplayTag("State.Mana.High"));
        info.cooldownSeconds = 5.0f;
        return info;
    }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
    }
};

TEST_CASE("Ability Inspector UI Projection", "[ability][editor]") {
    AbilitySystemComponent asc;
    AbilityInspectorPanel panel;

    FireboltAbility firebol;
    asc.addTag(GameplayTag("State.Mana.High"));
    asc.addTag(GameplayTag("Buff.Haste"));

    bool success = asc.tryActivateAbility(firebol);
    REQUIRE(success);
    REQUIRE(asc.getCooldownRemaining("Firebolt") > 0.0f);

    panel.update(asc);
    panel.render();

    asc.update(2.5f);
    panel.update(asc);
    panel.render();

    asc.update(3.0f);
    panel.update(asc);
    panel.render();

    REQUIRE(asc.getCooldownRemaining("Firebolt") == 0.0f);
}
