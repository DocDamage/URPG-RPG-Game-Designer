#include "engine/core/progression/class_progression.h"
#include "engine/core/progression/skill_tree.h"
#include "editor/progression/skill_tree_panel.h"

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
