#include "editor/battle/battle_presentation_panel.h"
#include "editor/battle/boss_designer_panel.h"
#include "editor/battle/formula_debugger_panel.h"
#include "engine/core/battle/enemy_ai_profile.h"
#include "engine/core/battle/party_tactics_profile.h"

#include <catch2/catch_test_macros.hpp>
#include <algorithm>

TEST_CASE("battle authoring validates battlebacks, HUD elements, and deterministic cue replay", "[battle][authoring][ffs05]") {
    urpg::battle::BattlePresentationProfile profile;
    profile.id = "arena";
    profile.battleback1 = "img/battlebacks1/CrystalCave.png";
    profile.battleback2 = "img/battlebacks2/MissingFog.png";
    profile.hud_elements = {
        {"hp", "gauge", 8, 8, true},
        {"state", "state_icon", 8, 32, true},
        {"turns", "turn_order", 160, 8, true},
        {"popup", "damage_popup", 200, 80, true},
        {"guard", "guard_marker", 64, 64, true},
    };
    profile.cue_timeline = {
        {"victory", "victory", 90, "me_victory"},
        {"cast", "cast", 10, "skill_1"},
        {"hit", "hit", 30, "slash"},
    };

    urpg::editor::BattlePresentationPanel panel;
    panel.loadProfile(profile, {"img/battlebacks1/CrystalCave.png"});
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().hud_element_count == 5);
    REQUIRE(panel.snapshot().cue_count == 3);
    REQUIRE(panel.snapshot().replay_cue_count == 3);
    REQUIRE(panel.validation().replay_cues[0].id == "cast");
    REQUIRE(panel.validation().replay_cues[1].id == "hit");
    REQUIRE(panel.validation().replay_cues[2].id == "victory");
    REQUIRE_FALSE(panel.validation().diagnostics.empty());
    REQUIRE(panel.validation().diagnostics[0].code == "missing_battleback");
}

TEST_CASE("battle authoring requires an explicit release battleback", "[battle][authoring][assets]") {
    urpg::battle::BattlePresentationProfile profile;
    profile.id = "arena_without_background";

    const auto result = urpg::battle::ValidateBattlePresentationProfile(profile, {});

    REQUIRE(std::any_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "required_battleback" &&
               diagnostic.severity == urpg::battle::BattleAuthoringSeverity::Error;
    }));
    REQUIRE(std::any_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "missing_battleback" &&
               diagnostic.severity == urpg::battle::BattleAuthoringSeverity::Error;
    }));
}

TEST_CASE("Boss profile validates phase threshold ordering", "[battle][authoring][ffs05]") {
    urpg::battle::BossProfile profile;
    profile.id = "lich";
    profile.phases = {
        {"phase_1", 75, {}, false, "You dare?", "", "bgm_phase1"},
        {"phase_2", 80, {"bone_guard"}, true, "Enough.", "crown", "bgm_phase2"},
    };

    urpg::editor::BossDesignerPanel panel;
    panel.loadProfile(profile);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().phase_count == 2);
    REQUIRE(panel.snapshot().diagnostic_count >= 1);
    REQUIRE(panel.validation().diagnostics[0].code == "phase_threshold_order");
}

TEST_CASE("Formula debugger uses bounded combat formula contract and reports fallbacks", "[battle][authoring][ffs05]") {
    urpg::editor::FormulaDebuggerPanel panel;
    panel.loadCases({
        {"supported", "a.atk * 4 - b.def * 2"},
        {"unsupported", "a.level * 2"},
        {"malformed", "a.atk +"},
    });
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().probe_count == 3);
    REQUIRE(panel.snapshot().fallback_count == 2);
    REQUIRE(panel.results()[0].id == "malformed");
    REQUIRE(panel.results()[0].reason == "malformed_formula_expression");
    REQUIRE(panel.results()[2].id == "unsupported");
    REQUIRE(panel.results()[2].reason == "unsupported_formula_symbol:a.level");
}

TEST_CASE("Enemy AI chooses deterministic weighted actions and rejects zero-weight profiles", "[battle][authoring][ffs05]") {
    urpg::battle::EnemyAiProfile profile;
    profile.id = "slime";
    profile.actions = {
        {"tackle", 10, ""},
        {"acid", 5, "enraged"},
        {"skip", 0, ""},
    };

    const urpg::battle::EnemyAiState normal_state;
    const auto normal = urpg::battle::ChooseEnemyAiAction(profile, normal_state, 4);
    REQUIRE(normal.has_value());
    REQUIRE(*normal == "tackle");

    const urpg::battle::EnemyAiState enraged_state{{"enraged"}};
    const auto enraged_a = urpg::battle::ChooseEnemyAiAction(profile, enraged_state, 12);
    const auto enraged_b = urpg::battle::ChooseEnemyAiAction(profile, enraged_state, 12);
    REQUIRE(enraged_a == enraged_b);
    REQUIRE(enraged_a.has_value());

    profile.actions = {{"skip", 0, ""}};
    REQUIRE_FALSE(urpg::battle::ChooseEnemyAiAction(profile, normal_state, 1).has_value());
}

TEST_CASE("Party tactics heal below threshold and defend when no heal is possible", "[battle][authoring][ffs05]") {
    urpg::battle::PartyTacticsProfile profile;
    profile.heal_below_percent = 40;
    profile.heal_action = "first_aid";
    profile.attack_action = "strike";
    profile.defend_action = "brace";

    const auto heal = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 20, 100, true});
    REQUIRE(heal.action_id == "first_aid");
    REQUIRE(heal.reason == "hp_below_threshold");

    const auto defend = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 20, 100, false});
    REQUIRE(defend.action_id == "brace");
    REQUIRE(defend.reason == "low_hp_no_heal_available");

    const auto attack = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 80, 100, false});
    REQUIRE(attack.action_id == "strike");
    REQUIRE(attack.reason == "default_attack");
}
