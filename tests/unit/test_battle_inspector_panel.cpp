#include "editor/battle/battle_inspector_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Battle inspector panel refreshes runtime flow, queue, and preview", "[battle][editor][panel]") {
    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    flow.enterAction();
    flow.noteEscapeFailure();

    urpg::battle::BattleActionQueue queue;
    queue.enqueue({"actor_a", "enemy_0", "attack", 120, 0});
    queue.enqueue({"actor_b", "enemy_0", "", 95, 0});

    urpg::editor::BattleInspectorPanel panel;
    panel.bindRuntime(flow, queue);
    panel.refresh();

    REQUIRE(panel.getModel().Summary().phase == "action");
    REQUIRE(panel.getModel().Summary().total_actions == 2);
    REQUIRE(panel.getModel().Summary().issue_count >= 1);
    REQUIRE(panel.getModel().VisibleRows().size() == 2);

    const auto& preview = panel.previewPanel().snapshot();
    REQUIRE(preview.phase == "action");
    REQUIRE(preview.physical_damage > 0);
    REQUIRE(preview.escape_ratio_next_fail >= preview.escape_ratio_now);

    panel.setShowIssuesOnly(true);
    panel.update();
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].subject_id == "actor_b");

    panel.setShowIssuesOnly(false);
    panel.setSubjectFilter(std::string("actor_a"));
    panel.update();
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].subject_id == "actor_a");

    panel.clearRuntime();
    panel.refresh();
    REQUIRE(panel.getModel().VisibleRows().empty());
}
