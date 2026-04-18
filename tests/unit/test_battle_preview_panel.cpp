#include "editor/battle/battle_preview_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Battle preview panel projects deterministic damage and escape previews", "[battle][editor][preview]") {
    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    flow.enterAction();
    flow.noteEscapeFailure();

    urpg::editor::BattlePreviewPanel panel;
    panel.bindRuntime(flow);

    urpg::battle::BattleDamageContext physical;
    physical.subject.atk = 20;
    physical.target.def = 10;
    physical.target.hp = 200;
    physical.power = 12;
    panel.setPhysicalPreviewContext(physical);

    urpg::battle::BattleDamageContext magical;
    magical.subject.mat = 28;
    magical.target.mdf = 8;
    magical.target.hp = 200;
    magical.power = 16;
    magical.magical = true;
    panel.setMagicalPreviewContext(magical);
    panel.setEscapePreviewAgility(80, 100);
    panel.refresh();

    const auto& snapshot = panel.snapshot();
    REQUIRE(snapshot.phase == "action");
    REQUIRE(snapshot.can_escape);
    REQUIRE(snapshot.physical_damage > 0);
    REQUIRE(snapshot.guarded_damage < snapshot.physical_damage);
    REQUIRE(snapshot.critical_damage > snapshot.physical_damage);
    REQUIRE(snapshot.magical_damage > 0);
    REQUIRE(snapshot.escape_ratio_next_fail > snapshot.escape_ratio_now);
    REQUIRE(snapshot.issue_count == panel.issues().size());
}

TEST_CASE("Battle preview panel emits guardrails for clamped escape agility", "[battle][editor][preview]") {
    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    flow.enterAction();
    flow.noteEscapeFailure();
    flow.markVictory();

    urpg::editor::BattlePreviewPanel panel;
    panel.bindRuntime(flow);
    panel.setEscapePreviewAgility(0, 0);
    panel.refresh();

    REQUIRE(panel.snapshot().phase == "victory");
    REQUIRE(panel.snapshot().issue_count >= 1);
}
