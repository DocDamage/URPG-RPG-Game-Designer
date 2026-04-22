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

TEST_CASE("Ability Inspector Diagnostics Snapshot", "[ability][editor]") {
    AbilitySystemComponent asc;
    AbilityInspectorPanel panel;

    FireboltAbility firebolt;
    asc.addTag(GameplayTag("State.Mana.High"));
    asc.grantAbility(std::make_shared<FireboltAbility>(firebolt));

    SECTION("Diagnostics snapshot is coherent before activation") {
        auto snap = panel.getDiagnosticsSnapshot(asc);
        REQUIRE(snap.ability_count == 1);
        REQUIRE(snap.active_cooldown_count == 0);
        REQUIRE(snap.last_execution_sequence_id == 0);
        REQUIRE(snap.ability_states.size() == 1);
        REQUIRE(snap.ability_states[0].id == "Firebolt");
        REQUIRE(snap.ability_states[0].can_activate);
        REQUIRE(snap.ability_states[0].blocking_reason.empty());
    }

    SECTION("Diagnostics snapshot reflects cooldown and history after activation") {
        asc.tryActivateAbility(firebolt);
        panel.update(asc);

        auto snap = panel.getDiagnosticsSnapshot(asc);
        REQUIRE(snap.ability_count == 1);
        REQUIRE(snap.active_cooldown_count == 1);
        REQUIRE(snap.last_execution_sequence_id == 1);
        REQUIRE(snap.ability_states.size() == 1);
        REQUIRE(snap.ability_states[0].id == "Firebolt");
        REQUIRE_FALSE(snap.ability_states[0].can_activate);
        REQUIRE(snap.ability_states[0].cooldown_remaining > 0.0f);
        REQUIRE(snap.ability_states[0].blocking_reason.find("Cooldown") != std::string::npos);
    }

    SECTION("Diagnostics snapshot reflects cooldown decay") {
        asc.tryActivateAbility(firebolt);
        asc.update(5.0f);
        panel.update(asc);

        auto snap = panel.getDiagnosticsSnapshot(asc);
        REQUIRE(snap.active_cooldown_count == 0);
        REQUIRE(snap.ability_states[0].can_activate);
        REQUIRE(snap.ability_states[0].blocking_reason.empty());
    }
}
