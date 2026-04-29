#include "engine/core/progression/class_progression.h"
#include "engine/core/progression/skill_tree.h"
#include "editor/progression/skill_tree_panel.h"
#include "engine/core/progression/stat_allocation.h"
#include "editor/progression/stat_allocation_panel.h"

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

TEST_CASE("Class progression detects prerequisite cycles and reports learned skills", "[progression][balance][ffs12]") {
    urpg::progression::ClassProgression graph;
    graph.addClass({"class.a", {"class.b"}, {{"skill.a", 2}}, {{1, 100}, {2, 120}}});
    graph.addClass({"class.b", {"class.a"}, {{"skill.b", 3}}, {{1, 90}}});

    REQUIRE(graph.validate().front().code == "class_graph_cycle");

    urpg::progression::ClassProgression acyclic;
    acyclic.addClass({"class.mage", {}, {{"skill.fire", 2}, {"skill.ice", 4}}, {{1, 40}, {2, 55}}});
    const auto skills = acyclic.learnedSkills("class.mage", 3);
    REQUIRE(skills.size() == 1);
    REQUIRE(skills[0] == "skill.fire");
}

TEST_CASE("Skill tree unlocks abilities and exposes editor preview", "[progression][skill_tree]") {
    const auto fixturePath = sourceRootFromMacro() / "content" / "fixtures" / "skill_tree_fixture.json";
    std::ifstream input(fixturePath);
    REQUIRE(input.is_open());
    nlohmann::json fixture;
    input >> fixture;

    const auto tree = urpg::progression::SkillTreeDocument::fromJson(fixture);
    const std::set<std::string> knownAbilities{"skill.fire", "skill.flame_wall"};
    REQUIRE(tree.validate(knownAbilities).empty());

    urpg::progression::SkillTreeState state;
    state.available_points = 2;

    auto preview = tree.preview(state, knownAbilities);
    REQUIRE(preview.unlockable_node_ids == std::vector<std::string>{"firebolt"});
    REQUIRE(preview.locked_node_ids == std::vector<std::string>{"flame_wall"});

    REQUIRE(tree.unlockNode("firebolt", state, knownAbilities));
    REQUIRE(state.available_points == 1);
    REQUIRE(state.learned_nodes.contains("firebolt"));

    preview = tree.preview(state, knownAbilities);
    REQUIRE(preview.learned_ability_ids == std::vector<std::string>{"skill.fire"});
    REQUIRE(preview.unlockable_node_ids == std::vector<std::string>{"flame_wall"});

    urpg::editor::SkillTreePanel panel;
    panel.bindDocument(tree);
    panel.setKnownAbilities(knownAbilities);
    panel.setState(state);
    REQUIRE(panel.selectNode("flame_wall"));
    REQUIRE(panel.unlockSelectedNode());
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "skill_tree");
    REQUIRE(snapshot["tree_id"] == "mage_fire");
    REQUIRE(snapshot["available_points"] == 0);
    REQUIRE(snapshot["selected_node"]["ability_id"] == "skill.flame_wall");
    REQUIRE(snapshot["preview"]["learned_ability_ids"].size() == 2);
}

TEST_CASE("Skill tree reports missing ability and prerequisite diagnostics", "[progression][skill_tree]") {
    const auto tree = urpg::progression::SkillTreeDocument::fromJson(nlohmann::json{
        {"tree_id", "broken"},
        {"nodes",
         {{{"id", "node_a"},
           {"title", "Node A"},
           {"ability_id", "missing.ability"},
           {"prerequisites", {"missing_node"}},
           {"cost", {{"currency", "skill_point"}, {"amount", 1}}}}}},
    });

    const auto diagnostics = tree.validate(std::set<std::string>{"known.ability"});
    REQUIRE(diagnostics.size() == 2);
    REQUIRE(diagnostics[0].code == "missing_ability_reference");
    REQUIRE(diagnostics[1].code == "missing_prerequisite");
}

TEST_CASE("Stat allocation previews level-up spending and editor state", "[progression][stat_allocation]") {
    urpg::progression::StatAllocationDocument document;
    document.addPool({"actor_hero_pool",
                      "actor.hero",
                      "class.warrior",
                      3,
                      5,
                      {{"hp", 1, 10, 180}, {"atk", 2, 1, 25}, {"agi", 1, 2, 18}}});

    REQUIRE(document.validate().empty());

    urpg::progression::ActorStatBlock stats;
    stats.hp = 150;
    stats.atk = 20;
    stats.agi = 10;

    urpg::progression::StatAllocationRequest request;
    request.pool_id = "actor_hero_pool";
    request.points_by_stat = {{"hp", 2}, {"atk", 1}, {"agi", 1}};

    const auto preview = document.preview("actor_hero_pool", stats, request);
    REQUIRE(preview.diagnostics.empty());
    REQUIRE(preview.spent_points == 5);
    REQUIRE(preview.remaining_points == 0);
    REQUIRE(preview.after.hp == 170);
    REQUIRE(preview.after.atk == 21);
    REQUIRE(preview.after.agi == 12);

    urpg::editor::StatAllocationPanel panel;
    panel.bindDocument(document);
    panel.setCurrentStats(stats);
    panel.setRequest(request);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "stat_allocation");
    REQUIRE(snapshot["pool_id"] == "actor_hero_pool");
    REQUIRE(snapshot["remaining_points"] == 0);
    REQUIRE(snapshot["after"]["hp"] == 170);

    const auto committed = document.commit("actor_hero_pool", stats, request);
    REQUIRE(committed.valid);
    REQUIRE(committed.actor_id == "actor.hero");
    REQUIRE(committed.class_id == "class.warrior");
    REQUIRE(committed.spent_points == 5);
    REQUIRE(committed.after.hp == 170);

    nlohmann::json save_document = {{"_urpg_format_version", "1.0"}};
    std::vector<std::string> save_diagnostics;
    REQUIRE(urpg::progression::attachStatAllocationToSaveDocument(save_document, committed, &save_diagnostics));
    REQUIRE(save_diagnostics.empty());
    REQUIRE(save_document[urpg::progression::kStatAllocationsSaveKey].size() == 1);
    REQUIRE(save_document[urpg::progression::kStatAllocationsSaveKey][0]["actorId"] == "actor.hero");
    REQUIRE(save_document[urpg::progression::kStatAllocationsSaveKey][0]["after"]["hp"] == 170);

    const auto loaded = urpg::progression::loadStatAllocationsFromSaveDocument(save_document, &save_diagnostics);
    REQUIRE(save_diagnostics.empty());
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].valid);
    REQUIRE(loaded[0].points_by_stat.at("atk") == 1);
    REQUIRE(loaded[0].after.agi == 12);
}

TEST_CASE("Stat allocation reports overspend missing rules and invalid pools", "[progression][stat_allocation]") {
    urpg::progression::StatAllocationDocument document;
    document.addPool({"bad_pool", "", "class.mage", -1, 1, {{"atk", 0, 1, 10}, {"atk", 1, 1, 10}, {"unknown", 1, 1, 10}}});
    auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 4);

    urpg::progression::StatAllocationDocument valid;
    valid.addPool({"pool", "actor.hero", "class.mage", 2, 1, {{"mat", 1, 2, 12}}});
    urpg::progression::ActorStatBlock stats;
    stats.mat = 10;
    urpg::progression::StatAllocationRequest request;
    request.pool_id = "pool";
    request.points_by_stat = {{"mat", 2}, {"luk", 1}};

    const auto preview = valid.preview("pool", stats, request);
    REQUIRE(preview.spent_points == 2);
    REQUIRE(preview.remaining_points == -1);
    REQUIRE(preview.after.mat == 12);
    REQUIRE(std::any_of(preview.diagnostics.begin(), preview.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "stat_points_overspent";
    }));
    REQUIRE(std::any_of(preview.diagnostics.begin(), preview.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "missing_stat_rule";
    }));
}
