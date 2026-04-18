#include "editor/battle/battle_inspector_model.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Battle inspector model builds deterministic action rows and summary", "[battle][editor][model]") {
    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    flow.enterAction();
    flow.noteEscapeFailure();

    urpg::battle::BattleActionQueue queue;
    queue.enqueue({"actor_b", "enemy_0", "attack", 110, 1});
    queue.enqueue({"", "enemy_0", "attack", 90, 0});
    queue.enqueue({"actor_a", "enemy_0", "attack", 120, 0});
    queue.enqueue({"actor_a", "enemy_0", "attack", 120, 0});

    urpg::editor::BattleInspectorModel model;
    model.LoadFromRuntime(flow, queue);

    const auto& summary = model.Summary();
    REQUIRE(summary.phase == "action");
    REQUIRE(summary.active);
    REQUIRE(summary.can_escape);
    REQUIRE(summary.turn_count == 0);
    REQUIRE(summary.escape_failures == 1);
    REQUIRE(summary.total_actions == 4);
    REQUIRE(summary.unique_subjects == 2);
    REQUIRE(summary.fastest_speed == 120);
    REQUIRE(summary.slowest_speed == 90);
    REQUIRE(summary.issue_count >= 2);

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 4);
    REQUIRE(rows[0].subject_id == "actor_a");
    REQUIRE(rows[0].speed == 120);
    REQUIRE(rows[1].subject_id == "actor_a");
    REQUIRE(rows[2].subject_id == "actor_b");
    REQUIRE(rows[3].subject_id.empty());
    REQUIRE(rows[3].issue_count >= 1);

    const auto& issues = model.Issues();
    REQUIRE_FALSE(issues.empty());
}

TEST_CASE("Battle inspector model supports subject and issue filters", "[battle][editor][model]") {
    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    flow.enterAction();

    urpg::battle::BattleActionQueue queue;
    queue.enqueue({"actor_a", "enemy_0", "attack", 120, 0});
    queue.enqueue({"actor_b", "enemy_0", "", 100, 0});

    urpg::editor::BattleInspectorModel model;
    model.LoadFromRuntime(flow, queue);

    model.SetSubjectFilter(std::string("actor_b"));
    auto rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].subject_id == "actor_b");

    model.SetSubjectFilter(std::nullopt);
    model.SetShowIssuesOnly(true);
    rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].subject_id == "actor_b");

    REQUIRE(model.SelectRow(0));
    const auto selected = model.SelectedSubjectId();
    REQUIRE(selected.has_value());
    REQUIRE(*selected == "actor_b");

    REQUIRE_FALSE(model.SelectRow(9));
    REQUIRE_FALSE(model.SelectedSubjectId().has_value());
}
