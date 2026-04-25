#include "engine/core/progression/class_progression.h"

#include <catch2/catch_test_macros.hpp>

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
