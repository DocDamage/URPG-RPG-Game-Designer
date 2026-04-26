#include "editor/battle/battle_inspector_panel.h"
#include "engine/core/scene/battle_scene.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::scene::BattleParticipant* findParticipantById(std::vector<urpg::scene::BattleParticipant>& participants,
                                                    const std::string& id,
                                                    bool isEnemy) {
    for (auto& participant : participants) {
        if (participant.id == id && participant.isEnemy == isEnemy) {
            return &participant;
        }
    }
    return nullptr;
}

} // namespace

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

    REQUIRE(panel.lastRenderSnapshot().status == "error");
    REQUIRE(panel.lastRenderSnapshot().runtime_bound);
    REQUIRE(panel.lastRenderSnapshot().visible_row_count == 2);
    REQUIRE(panel.lastRenderSnapshot().issue_count >= 1);
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
    REQUIRE(panel.lastRenderSnapshot().status == "disabled");
    REQUIRE_FALSE(panel.lastRenderSnapshot().runtime_bound);
    REQUIRE(panel.getModel().VisibleRows().empty());
}

TEST_CASE("Battle inspector panel surfaces disabled and empty render states", "[battle][editor][panel][empty]") {
    urpg::editor::BattleInspectorPanel panel;
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.status == "disabled");
    REQUIRE(snapshot.can_refresh == false);
    REQUIRE_FALSE(snapshot.remediation.empty());

    urpg::battle::BattleFlowController flow;
    flow.beginBattle(true);
    urpg::battle::BattleActionQueue queue;
    panel.bindRuntime(flow, queue);
    panel.refresh();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.status == "empty");
    REQUIRE(snapshot.runtime_bound);
    REQUIRE(snapshot.can_refresh);
    REQUIRE(snapshot.visible_row_count == 0);
    REQUIRE(snapshot.message == "No battle actions are queued.");
    REQUIRE(snapshot.remediation.find("Queue a battle action") != std::string::npos);
}

TEST_CASE("Battle inspector panel binds live scene diagnostics preview payload", "[battle][editor][panel]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    urpg::scene::BattleScene runtime({"2"});
    runtime.onStart();
    runtime.addActor("1", "Hero", 100, 30, {0.0f, 0.0f}, nullptr);
    runtime.addEnemy("2", "Goblin", 50, 0, {100.0f, 100.0f}, nullptr);
    runtime.setPhase(urpg::scene::BattlePhase::ACTION);
    runtime.flowController().noteEscapeFailure();

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(runtime.getParticipants());
    auto* hero = findParticipantById(participants, "1", false);
    auto* goblin = findParticipantById(participants, "2", true);
    REQUIRE(hero != nullptr);
    REQUIRE(goblin != nullptr);

    urpg::scene::BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = goblin;
    action.command = "attack";
    runtime.addActionToQueue(action);

    const auto preview = runtime.buildDiagnosticsPreview();
    REQUIRE(preview.has_value());

    const auto expected_physical_damage = urpg::battle::BattleRuleResolver::resolveDamage(preview->physical_preview);
    const auto expected_magical_damage = urpg::battle::BattleRuleResolver::resolveDamage(preview->magical_preview);
    const auto expected_escape_ratio_now = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview->party_agi, preview->troop_agi, runtime.flowController().escapeFailures());
    const auto expected_escape_ratio_next = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview->party_agi, preview->troop_agi, runtime.flowController().escapeFailures() + 1);

    urpg::editor::BattleInspectorPanel panel;
    panel.bindRuntime(runtime);
    panel.refresh();

    REQUIRE(panel.getModel().Summary().phase == "action");
    REQUIRE(panel.getModel().Summary().total_actions == 1);
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].subject_id == hero->id);

    const auto& snapshot = panel.previewPanel().snapshot();
    REQUIRE(snapshot.phase == "action");
    REQUIRE(snapshot.can_escape == true);
    REQUIRE(snapshot.physical_damage == expected_physical_damage);
    REQUIRE(snapshot.magical_damage == expected_magical_damage);
    REQUIRE(snapshot.escape_ratio_now == expected_escape_ratio_now);
    REQUIRE(snapshot.escape_ratio_next_fail == expected_escape_ratio_next);
}
