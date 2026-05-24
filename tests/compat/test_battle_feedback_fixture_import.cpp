#include "engine/core/battle/battle_core.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Compat fixture import: battle feedback policy payloads produce native evidence",
          "[compat][fixtures][battle_feedback]") {
    const auto imported = urpg::battle::BattleRuleResolver::importFeedbackPolicyFixture(nlohmann::json{
        {"name", "CompatBattleFeedbackFixture"},
        {"parameters",
         {
             {"Chip Damage Percent", "20"},
             {"Chip Healing Percent", "35"},
             {"Minimum Chip Damage", "2"},
             {"Minimum Chip Healing", "4"},
             {"Custom Buff Levels", "6"},
             {"Zero Damage Presentation", "zero_as_evasion"},
             {"Reuse Troop Positions", "true"},
         }},
    });

    REQUIRE(imported.imported);
    REQUIRE(imported.policy.chip_damage_percent == 20);
    REQUIRE(imported.policy.chip_healing_percent == 35);
    REQUIRE(imported.policy.min_chip_damage == 2);
    REQUIRE(imported.policy.min_chip_healing == 4);
    REQUIRE(imported.policy.max_buff_level == 6);
    REQUIRE(imported.policy.zero_damage_policy == urpg::battle::ZeroDamagePresentationPolicy::Evasion);
    REQUIRE(imported.policy.reuse_troop_positions);
    REQUIRE(imported.coverage_rows.size() == 5);
    REQUIRE(std::all_of(imported.coverage_rows.begin(), imported.coverage_rows.end(),
                        [](const auto& row) { return row.covered; }));
    REQUIRE(imported.diagnostics[0].code == "feedback_fixture_imported");
    REQUIRE(imported.diagnostics[0].target == "CompatBattleFeedbackFixture");
}
