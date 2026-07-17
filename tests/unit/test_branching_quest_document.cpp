#include "engine/core/narrative/branching_quest_document.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::narrative::BranchingQuestDocument makeBranchingQuest() {
    urpg::narrative::BranchingQuestDocument document;
    document.id = "quest.echoes";
    document.dialogue.addNode({"start", "guide", "Guide", "dialogue.guide.start", "Will you help?", false, {},
                               "voice.guide.start", "caption.guide.start", 0, 0, true, {}, 0, 0, {}});
    document.dialogue.addNode({"accept", "guide", "Guide", "dialogue.guide.accept", "Find the echo.", true, {},
                               "voice.guide.accept", "caption.guide.accept", 320, 0, true, {}, 0, 0, {}});
    document.dialogue.addNode({"decline", "guide", "Guide", "dialogue.guide.decline", "Another time.", true, {},
                               "voice.guide.decline", "caption.guide.decline", 320, 180, true, {}, 0, 0, {}});
    document.dialogue.addChoice("start", {"decline_quest", "No", "decline", {}, {}, "choice.quest.decline"});
    document.dialogue.addChoice("start", {"accept_quest", "Yes", "accept", {{"reputation", ">=", 2}},
                                           {{"quest_started", 1}}, "choice.quest.accept"});
    document.dialogue.setStartNode("start");

    document.quest.quest_id = "quest.echoes";
    document.quest.title = "Echoes in Stone";
    document.quest.addNode({"start", "start", "Start", "", "quest.echoes.start", {}, {}, 0, 0, false});
    document.quest.addNode({"find_echo", "objective", "Find the echo", "find_echo", "quest.echoes.find",
                            {{"dialogue_choice", "accept_quest", 1}}, {{"item", "echo_shard", 1}}, 0, 0, false});
    document.quest.addNode({"complete", "complete", "Complete", "", "quest.echoes.complete", {},
                            {{"gold", "gold", 250}}, 0, 0, false});
    document.quest.connect("start", "find_echo");
    document.quest.connect("find_echo", "complete");
    return document;
}

} // namespace

TEST_CASE("branching voiced quest edits and round trips through runtime save and package",
          "[narrative][branching_quest][pcq404]") {
    auto document = makeBranchingQuest();
    REQUIRE(document.validate().empty());

    REQUIRE(document.dialogue.reorderChoice("start", "accept_quest", 0));
    REQUIRE(document.quest.updateNodeCanvasPosition("find_echo", 200, 120));
    REQUIRE(document.quest.reorderNode("complete", 1));
    REQUIRE(document.quest.disconnect("find_echo", "complete"));
    REQUIRE(document.quest.connect("find_echo", "complete"));

    const auto packaged = document.toJson();
    REQUIRE(packaged["package_closure"]["voice_asset_ids"].size() == 3);
    REQUIRE(packaged["package_closure"]["localization_keys"].size() == 11);
    REQUIRE(packaged["package_closure"]["quest_ids"] == nlohmann::json::array({"quest.echoes"}));

    bool migrated = true;
    auto restored = urpg::narrative::BranchingQuestDocument::fromJson(packaged, &migrated);
    REQUIRE(restored);
    REQUIRE_FALSE(migrated);
    REQUIRE(restored->dialogue.findNode("start")->choices.front().id == "accept_quest");
    REQUIRE(restored->quest.nodes[1].id == "complete");
    REQUIRE(restored->quest.nodes[2].canvas_x == 200);

    urpg::narrative::BranchingQuestRuntime runtime;
    REQUIRE(runtime.start(*restored));
    runtime.dialogue_values["reputation"] = 2;
    REQUIRE(runtime.choose(*restored, "accept_quest"));
    const auto progress = runtime.advanceQuest(*restored, {}, "2026-07-16T12:00:00Z");
    REQUIRE(progress.completed_objective_ids == std::vector<std::string>{"find_echo"});

    const auto save = runtime.save();
    const auto loaded = urpg::narrative::BranchingQuestRuntime::load(save);
    REQUIRE(loaded);
    REQUIRE(loaded->dialogue_node_id == "accept");
    REQUIRE(loaded->dialogue_values.at("quest_started") == 1);
    REQUIRE(loaded->quest_registry.findQuest("quest.echoes")->objectives.front().state ==
            urpg::quest::ObjectiveState::Completed);
}

TEST_CASE("branching quest migrates legacy editor documents and reports broken cross references",
          "[narrative][branching_quest][migration][pcq404]") {
    auto document = makeBranchingQuest();
    auto legacy = nlohmann::json{{"quest_id", document.id},
                                 {"dialogue", document.dialogue.serialize()},
                                 {"quest", document.quest.toJson()}};
    bool migrated = false;
    auto restored = urpg::narrative::BranchingQuestDocument::fromJson(legacy, &migrated);
    REQUIRE(restored);
    REQUIRE(migrated);
    REQUIRE(restored->toJson()["schema_version"] == "urpg.branching_quest.v2");

    restored->quest.nodes[1].conditions[0].id = "deleted_choice";
    const auto diagnostics = restored->validate();
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "missing_dialogue_choice" && diagnostic.source_id == "find_echo";
    }));
    REQUIRE_FALSE(urpg::narrative::BranchingQuestDocument::fromJson(nlohmann::json::object()));
}
