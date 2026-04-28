#include "engine/core/quest/quest_registry.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/quest/quest_validator.h"
#include "editor/quest/quest_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

} // namespace

TEST_CASE("quest objective advancement is deterministic and persists", "[quest][narrative][ffs10]") {
    urpg::quest::QuestRegistry registry;
    REQUIRE(registry.registerQuest({
        "intro",
        {{"meet_guide",
          urpg::quest::ObjectiveState::Locked,
          {{"switch", "intro_started", 1}, {"reputation", "guide", 10}},
          ""}},
    }));

    urpg::quest::QuestWorldState world;
    world.switches["intro_started"] = true;
    world.reputation["guide"] = 12;

    REQUIRE(registry.evaluateObjective("intro", "meet_guide", world, "2026-04-25T12:00:00Z"));
    const auto* quest = registry.findQuest("intro");
    REQUIRE(quest != nullptr);
    REQUIRE(quest->objectives[0].state == urpg::quest::ObjectiveState::Completed);

    const auto restored = urpg::quest::QuestRegistry::deserialize(registry.serialize());
    const auto* restored_quest = restored.findQuest("intro");
    REQUIRE(restored_quest != nullptr);
    REQUIRE(restored_quest->objectives[0].state == urpg::quest::ObjectiveState::Completed);
    REQUIRE(restored_quest->objectives[0].updated_at == "2026-04-25T12:00:00Z");
}

TEST_CASE("quest validator reports deleted item references", "[quest][narrative][ffs10]") {
    const urpg::quest::QuestDefinition quest{
        "fetch",
        {{"find_relic", urpg::quest::ObjectiveState::Locked, {{"item", "deleted_relic", 1}}, ""}},
    };

    const urpg::quest::QuestValidator validator;
    const auto diagnostics = validator.validate(quest, {"potion"});

    REQUIRE(diagnostics == std::vector<std::string>{"missing_item_reference:deleted_relic"});
}

TEST_CASE("quest objective graph loads previews executes and round trips", "[quest][graph]") {
    const auto fixturePath = sourceRootFromMacro() / "content" / "fixtures" / "quest_objective_graph_fixture.json";
    std::ifstream input(fixturePath);
    REQUIRE(input.is_open());
    nlohmann::json fixture;
    input >> fixture;

    const auto graph = urpg::quest::QuestObjectiveGraphDocument::fromJson(fixture);
    REQUIRE(graph.quest_id == "intro_relic");
    REQUIRE(graph.nodes.size() == 4);
    REQUIRE(graph.links.size() == 3);
    REQUIRE(graph.validate().empty());

    urpg::quest::QuestWorldState world;
    world.switches["intro_started"] = true;
    world.items.push_back("ancient_relic");
    world.reputation["guide"] = 12;

    const auto preview = graph.preview(world);
    REQUIRE(preview.diagnostics.empty());
    REQUIRE(preview.ready_node_ids == std::vector<std::string>{"find_relic"});
    REQUIRE(preview.completed_objective_ids == std::vector<std::string>{"find_relic"});
    REQUIRE(preview.blocked_node_ids.empty());

    urpg::quest::QuestRegistry registry;
    const auto apply = graph.applyReadyObjectives(registry, world, "2026-04-28T12:00:00Z");
    REQUIRE(apply.completed_objective_ids == std::vector<std::string>{"find_relic"});

    const auto* quest = registry.findQuest("intro_relic");
    REQUIRE(quest != nullptr);
    REQUIRE(quest->objectives.size() == 1);
    REQUIRE(quest->objectives[0].state == urpg::quest::ObjectiveState::Completed);
    REQUIRE(quest->objectives[0].updated_at == "2026-04-28T12:00:00Z");

    const auto roundTrip = urpg::quest::QuestObjectiveGraphDocument::fromJson(graph.toJson());
    REQUIRE(roundTrip.quest_id == graph.quest_id);
    REQUIRE(roundTrip.nodes[1].rewards[0].id == "gold");
}

TEST_CASE("quest objective graph validates missing links and editor preview", "[quest][graph][editor]") {
    auto graph = urpg::quest::QuestObjectiveGraphDocument::fromJson(nlohmann::json{
        {"quest_id", "broken"},
        {"title", "Broken Quest"},
        {"nodes",
         {{{"id", "start"}, {"type", "start"}, {"title", "Start"}},
          {{"id", "objective_a"},
           {"type", "objective"},
           {"title", "Objective A"},
           {"conditions", {{{"type", "switch"}, {"id", "ready"}, {"value", 1}}}}}}},
        {"links", {{{"from", "start"}, {"to", "missing_node"}}}},
    });

    const auto diagnostics = graph.validate();
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics[0].code == "missing_link_target");
    REQUIRE(diagnostics[0].node_id == "missing_node");

    graph.links = {{"start", "objective_a"}};

    urpg::quest::QuestWorldState world;
    world.switches["ready"] = true;

    urpg::editor::QuestPanel panel;
    panel.bindObjectiveGraph(graph);
    panel.setPreviewWorldState(world);
    REQUIRE(panel.selectGraphNode("objective_a"));
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "quest");
    REQUIRE(snapshot["graph_bound"] == true);
    REQUIRE(snapshot["graph"]["quest_id"] == "broken");
    REQUIRE(snapshot["graph"]["selected_node_id"] == "objective_a");
    REQUIRE(snapshot["graph"]["selected_node"]["condition_count"] == 1);
    REQUIRE(snapshot["graph"]["preview"]["ready_node_ids"][0] == "objective_a");

    REQUIRE(panel.applyReadyGraphObjectives("2026-04-28T12:30:00Z"));
    panel.render();
    const auto appliedSnapshot = panel.lastRenderSnapshot();
    REQUIRE(appliedSnapshot["registry"]["quests"][0]["objectives"][0]["state"] == "completed");
}
