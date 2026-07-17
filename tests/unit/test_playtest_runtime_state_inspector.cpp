#include "editor/playtest/playtest_runtime_state_inspector.h"
#include "engine/core/playtest/playtest_runtime_state_owner.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

urpg::editor::PlaytestRuntimeState fixture() {
    urpg::editor::PlaytestRuntimeState state;
    state.switches = {{"door.open", false}};
    state.variables = {{"score", 7}};
    state.self_switches = {{"event.chest:A", false}};
    state.entities.emplace("enemy.wisp", urpg::editor::PlaytestEntityState{
        "enemy.wisp", "enemy", {{"hp", 30}, {"visible", true}}});
    state.quests.emplace("quest.wisp", urpg::editor::PlaytestQuestRuntimeState{
        "quest.wisp", "active", {{"objective.defeat", "active"}}});
    state.inventory = {{"item.potion", 1}};
    return state;
}

} // namespace

TEST_CASE("Playtest inspector watches every runtime state family and marks temporary edits",
          "[playtest][state_inspector][pcq502]") {
    urpg::editor::PlaytestRuntimeStateInspector inspector;
    REQUIRE(inspector.beginSession("checkpoint.launch", fixture()));
    const std::vector<urpg::editor::PlaytestDebugValueAddress> addresses{
        {urpg::editor::PlaytestDebugValueKind::Switch, "door.open", ""},
        {urpg::editor::PlaytestDebugValueKind::Variable, "score", ""},
        {urpg::editor::PlaytestDebugValueKind::SelfSwitch, "event.chest:A", ""},
        {urpg::editor::PlaytestDebugValueKind::EntityField, "enemy.wisp", "hp"},
        {urpg::editor::PlaytestDebugValueKind::QuestState, "quest.wisp", "objective.defeat"},
        {urpg::editor::PlaytestDebugValueKind::Inventory, "item.potion", ""},
    };
    for (const auto& address : addresses) REQUIRE(inspector.watch(address));
    REQUIRE_FALSE(inspector.watch(addresses.front()));
    const auto before = inspector.watchedValues();
    REQUIRE(before.size() == 6);
    for (const auto& watch : before) {
        REQUIRE(watch.available);
        REQUIRE_FALSE(watch.debug_modified);
    }

    REQUIRE(inspector.applyTemporaryEdit("edit.switch", addresses[0], true).success);
    REQUIRE(inspector.applyTemporaryEdit("edit.variable", addresses[1], 99).success);
    REQUIRE(inspector.applyTemporaryEdit("edit.self", addresses[2], true).success);
    REQUIRE(inspector.applyTemporaryEdit("edit.entity", addresses[3], 1).success);
    REQUIRE(inspector.applyTemporaryEdit("edit.quest", addresses[4], "completed").success);
    REQUIRE(inspector.applyTemporaryEdit("edit.inventory", addresses[5], 8).success);
    const auto after = inspector.watchedValues();
    for (const auto& watch : after) REQUIRE(watch.debug_modified);
    REQUIRE(after[0].value == true);
    REQUIRE(after[1].value == 99);
    REQUIRE(after[3].value == 1);
    REQUIRE(after[4].value == "completed");
    REQUIRE(after[5].value == 8);
}

TEST_CASE("Disposable playtest edits never enter package state and reset to checkpoint",
          "[playtest][state_inspector][pcq502]") {
    urpg::editor::PlaytestRuntimeStateInspector inspector;
    REQUIRE(inspector.beginSession("checkpoint.launch", fixture()));
    REQUIRE(inspector.applyTemporaryEdit("edit.score",
        {urpg::editor::PlaytestDebugValueKind::Variable, "score", ""}, 500).success);
    REQUIRE(inspector.state().variables.at("score") == 500);
    REQUIRE(inspector.packageState().variables.at("score") == 7);
    const auto overlay = inspector.exportDisposableOverlay();
    REQUIRE(overlay["schema"] == "urpg.playtest_debug_overlay.v1");
    REQUIRE(overlay["disposable"] == true);
    REQUIRE(overlay["packaged"] == false);
    REQUIRE(overlay["state"]["variables"]["score"] == 500);
    REQUIRE(overlay["package_state_unchanged"]["variables"]["score"] == 7);
    REQUIRE(overlay["mutations"][0]["temporary"] == true);
    REQUIRE(overlay["mutations"][0]["packaged"] == false);
    REQUIRE(inspector.resetToCheckpoint("checkpoint.launch"));
    REQUIRE(inspector.state().variables.at("score") == 7);
    REQUIRE(inspector.mutations().empty());
    REQUIRE_FALSE(inspector.resetToCheckpoint("checkpoint.missing"));
}

TEST_CASE("Playtest inspector validates temporary edit types ranges and addresses",
          "[playtest][state_inspector][pcq502]") {
    urpg::editor::PlaytestRuntimeStateInspector inspector;
    REQUIRE(inspector.beginSession("checkpoint.launch", fixture()));
    REQUIRE_FALSE(inspector.applyTemporaryEdit("bad.switch",
        {urpg::editor::PlaytestDebugValueKind::Switch, "door.open", ""}, 1).success);
    REQUIRE_FALSE(inspector.applyTemporaryEdit("bad.inventory",
        {urpg::editor::PlaytestDebugValueKind::Inventory, "item.potion", ""}, -1).success);
    REQUIRE_FALSE(inspector.applyTemporaryEdit("bad.entity",
        {urpg::editor::PlaytestDebugValueKind::EntityField, "enemy.wisp", "hp"},
        nlohmann::json::object()).success);
    REQUIRE_FALSE(inspector.applyTemporaryEdit("bad.quest",
        {urpg::editor::PlaytestDebugValueKind::QuestState, "quest.wisp", "state"}, "impossible").success);
    REQUIRE_FALSE(inspector.applyTemporaryEdit("bad.missing",
        {urpg::editor::PlaytestDebugValueKind::Variable, "missing", ""}, 1).success);
    REQUIRE(inspector.mutations().empty());
    REQUIRE(inspector.state().switches.at("door.open") == false);
    REQUIRE(inspector.state().inventory.at("item.potion") == 1);
}

TEST_CASE("Playtest inspector switches between explicit runtime checkpoints",
          "[playtest][state_inspector][pcq502]") {
    urpg::editor::PlaytestRuntimeStateInspector inspector;
    REQUIRE(inspector.beginSession("checkpoint.launch", fixture()));
    auto battle = fixture();
    battle.variables["score"] = 70;
    battle.inventory["item.potion"] = 2;
    battle.quests["quest.wisp"].state = "completed";
    REQUIRE(inspector.addCheckpoint("checkpoint.after_battle", battle));
    REQUIRE_FALSE(inspector.addCheckpoint("checkpoint.after_battle", battle));
    REQUIRE(inspector.resetToCheckpoint("checkpoint.after_battle"));
    REQUIRE(inspector.activeCheckpointId() == "checkpoint.after_battle");
    REQUIRE(inspector.state().variables.at("score") == 70);
    REQUIRE(inspector.state().inventory.at("item.potion") == 2);
    REQUIRE(inspector.state().quests.at("quest.wisp").state == "completed");
    REQUIRE(inspector.packageState().variables.at("score") == 7);
}

TEST_CASE("Live runtime state bridge acknowledges disposable edits and checkpoint reset",
          "[playtest][state_inspector][pcq502][runtime_bridge]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_live_runtime_state_inspector";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const nlohmann::json initial{
        {"switches", {{"door.open", false}}}, {"variables", {{"score", 7}}},
        {"self_switches", {{"event.chest:A", false}}},
        {"entities", {{"player", {{"type", "player"}, {"fields", {{"tile_x", 1}, {"tile_y", 2}}}}}}},
        {"quests", {{"quest.wisp", {{"state", "active"},
                                      {"objectives", {{"objective.defeat", "active"}}}}}}},
        {"inventory", {{"item.potion", 1}}},
    };
    urpg::playtest::PlaytestRuntimeStateOwner runtime(root);
    urpg::playtest::PlaytestRuntimeStateBridge editorBridge(root);
    urpg::editor::PlaytestRuntimeStateInspector inspector;
    REQUIRE(runtime.start("session-state", "launch", initial));
    inspector.bindLiveBridge(&editorBridge);
    REQUIRE(inspector.refreshLive());
    REQUIRE(inspector.liveConnected());
    REQUIRE(inspector.liveRevision() == 1);
    REQUIRE(inspector.packageState().variables.at("score") == 7);
    REQUIRE(inspector.watch({urpg::editor::PlaytestDebugValueKind::Variable, "score", {}}));

    REQUIRE(inspector.requestLiveTemporaryEdit(
        "edit.score", {urpg::editor::PlaytestDebugValueKind::Variable, "score", {}}, 99));
    REQUIRE(runtime.poll());
    REQUIRE(runtime.revision() == 2);
    REQUIRE(inspector.refreshLive());
    REQUIRE(inspector.state().variables.at("score") == 99);
    REQUIRE(inspector.lastLiveControlCode() == "runtime_state_temporary_edit_applied");
    REQUIRE(inspector.packageState().variables.at("score") == 7);
    REQUIRE(inspector.mutations().size() == 1);
    REQUIRE(inspector.mutations()[0].temporary);
    REQUIRE_FALSE(inspector.mutations()[0].packaged);
    REQUIRE(inspector.watchedValues()[0].debug_modified);

    auto moved = initial;
    moved["entities"]["player"]["fields"]["tile_x"] = 4;
    REQUIRE(runtime.synchronize(moved));
    REQUIRE(inspector.refreshLive());
    REQUIRE(inspector.state().entities.at("player").fields.at("tile_x") == 4);
    REQUIRE(inspector.state().variables.at("score") == 99);

    REQUIRE(inspector.requestLiveReset("launch"));
    REQUIRE(runtime.poll());
    REQUIRE(inspector.refreshLive());
    REQUIRE(inspector.state().variables.at("score") == 7);
    REQUIRE(inspector.mutations().empty());
    REQUIRE(inspector.lastLiveControlCode() == "runtime_state_checkpoint_reset_applied");
    REQUIRE(inspector.exportDisposableOverlay()["packaged"] == false);

    urpg::playtest::RuntimeStateControl stale;
    stale.control_id = 99;
    stale.expected_revision = 1;
    stale.action = urpg::playtest::RuntimeStateControlAction::TemporaryEdit;
    stale.mutation_id = "edit.stale";
    stale.kind = "variable";
    stale.id = "score";
    stale.value = 1000;
    REQUIRE(editorBridge.appendControl(stale));
    REQUIRE(runtime.poll());
    REQUIRE(inspector.refreshLive());
    REQUIRE(inspector.lastLiveControlCode() == "runtime_state_control_revision_rejected");
    REQUIRE(inspector.state().variables.at("score") == 7);
    std::filesystem::remove_all(root, error);
}
