#include <catch2/catch_test_macros.hpp>

#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"

#include <iostream>
#include <sstream>

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

    void activate(AbilitySystemComponent& source) override { commitAbility(source); }
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

    std::ostringstream capturedOutput;
    auto* originalOutput = std::cout.rdbuf(capturedOutput.rdbuf());

    panel.update(asc);
    panel.render();
    auto renderSnap = panel.getRenderSnapshot();
    REQUIRE(renderSnap.has_rendered_frame);
    REQUIRE(renderSnap.controls.size() == 6);

    asc.update(2.5f);
    panel.update(asc);
    panel.render();

    asc.update(3.0f);
    panel.update(asc);
    panel.render();

    REQUIRE(asc.getCooldownRemaining("Firebolt") == 0.0f);
    std::cout.rdbuf(originalOutput);
    REQUIRE(capturedOutput.str().empty());
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

    SECTION("Panel selection and preview activation keep the snapshot current") {
        panel.update(asc);
        REQUIRE(panel.selectAbility(0, asc));
        auto renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.selected_ability_id == "Firebolt");
        REQUIRE(renderSnap.selected_ability_can_activate == true);

        REQUIRE(panel.previewSelectedAbility(asc));
        renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.latest_ability_id == "Firebolt");
        REQUIRE(renderSnap.latest_outcome == "executed");
        REQUIRE(renderSnap.selected_ability_id == "Firebolt");
        REQUIRE(renderSnap.selected_ability_can_activate == false);
        REQUIRE(renderSnap.selected_ability_blocking_reason.find("Cooldown") != std::string::npos);
    }

    SECTION("Panel exposes wired command controls for the editor host") {
        REQUIRE(panel.applyDraftPatternPreset("skill_cross_small"));
        panel.update(asc);
        REQUIRE(panel.selectAbility(0, asc));

        panel.setCommandCallbacks({
            [] { return true; },
            [] { return true; },
            [] { return true; },
            [] { return true; },
        });
        panel.render();

        const auto renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.has_rendered_frame);
        REQUIRE(renderSnap.controls.size() == 6);

        auto controlEnabled = [&renderSnap](const std::string& id) {
            for (const auto& control : renderSnap.controls) {
                if (control.id == id) {
                    return control.enabled;
                }
            }
            return false;
        };

        auto disabledReason = [&renderSnap](const std::string& id) {
            for (const auto& control : renderSnap.controls) {
                if (control.id == id) {
                    return control.disabled_reason;
                }
            }
            return std::string{};
        };

        REQUIRE(controlEnabled("select_ability"));
        REQUIRE(controlEnabled("preview_selected"));
        REQUIRE(controlEnabled("validate_draft"));
        REQUIRE(controlEnabled("apply_draft"));
        REQUIRE(controlEnabled("save_draft"));
        REQUIRE(controlEnabled("load_draft"));
        REQUIRE(disabledReason("preview_selected").empty());
        REQUIRE(renderSnap.validation_issues.empty());
    }

    SECTION("Disabled command controls expose reviewer-visible reasons") {
        panel.update(asc);
        panel.render();

        const auto renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.controls.size() == 6);

        auto disabledReason = [&renderSnap](const std::string& id) {
            for (const auto& control : renderSnap.controls) {
                if (control.id == id) {
                    return control.disabled_reason;
                }
            }
            return std::string{};
        };

        REQUIRE(disabledReason("preview_selected") == "Select an ability before previewing.");
        REQUIRE(disabledReason("apply_draft") == "Editor host has not registered an apply-draft command handler.");
        REQUIRE(disabledReason("save_draft") == "Editor host has not registered a save-draft command handler.");
        REQUIRE(disabledReason("load_draft") == "Editor host has not registered a load-draft command handler.");
    }

    SECTION("Command feedback remains visible after panel refresh") {
        panel.update(asc);
        panel.recordCommandResult("save_draft", true, "Saved draft ability to content/abilities/skill_draft.json");
        panel.render();

        auto renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.latest_command_id == "save_draft");
        REQUIRE(renderSnap.latest_command_success);
        REQUIRE(renderSnap.latest_command_message == "Saved draft ability to content/abilities/skill_draft.json");

        panel.update(asc);
        renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.latest_command_id == "save_draft");
        REQUIRE(renderSnap.latest_command_success);
        REQUIRE(renderSnap.latest_command_message == "Saved draft ability to content/abilities/skill_draft.json");
    }

    SECTION("Draft ability preview builds a workspace-owned editable runtime") {
        AbilitySystemComponent draftAsc;
        REQUIRE(panel.setDraftAbilityId("skill.authored"));
        REQUIRE(panel.setDraftMpCost(6.0f));
        REQUIRE(panel.setDraftCooldownSeconds(4.0f));
        REQUIRE(panel.setDraftEffectId("effect.authored"));
        REQUIRE(panel.setDraftEffectAttribute("Defense"));
        REQUIRE(panel.setDraftEffectValue(14.0f));
        REQUIRE(panel.setDraftEffectDuration(8.0f));
        REQUIRE(panel.applyDraftPatternPreset("skill_cross_small"));

        panel.populateDraftPreviewRuntime(draftAsc);
        panel.update(draftAsc);
        REQUIRE(panel.selectDraftAbility(draftAsc));

        const auto renderSnap = panel.getRenderSnapshot();
        REQUIRE(renderSnap.draft_preview.has_draft);
        REQUIRE(renderSnap.draft_preview.ability_id == "skill.authored");
        REQUIRE(renderSnap.draft_preview.effect_attribute == "Defense");
        REQUIRE(renderSnap.draft_preview.preview_attribute_after == 114.0f);
        REQUIRE(renderSnap.draft_preview.pattern_preview.is_valid);
        REQUIRE(renderSnap.draft_preview.pattern_preview.grid_rows[2].find("[O]") != std::string::npos);
        REQUIRE(draftAsc.getAbilities().size() == 1);

        REQUIRE(panel.previewSelectedAbility(draftAsc));
        REQUIRE(draftAsc.getAttribute("MP", 0.0f) == 24.0f);
        REQUIRE(draftAsc.getAttribute("Defense", 0.0f) == 114.0f);
    }
}
