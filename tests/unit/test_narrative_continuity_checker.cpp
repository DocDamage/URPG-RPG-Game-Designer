#include "engine/core/narrative/narrative_continuity_checker.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("narrative continuity checker detects orphaned nodes unresolved choices and contradictory conditions", "[narrative][ffs10]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({
        "start",
        "guide",
        "Guide",
        "dialogue.start",
        "Start",
        false,
        {{"impossible", "Impossible", "missing", {{"trust", ">=", 10}, {"trust", "<=", 5}}, {}}},
    }));
    REQUIRE(graph.addNode({"orphan", "guide", "Guide", "", "Lost", false, {}}));

    const urpg::narrative::NarrativeContinuityChecker checker;
    const auto diagnostics = checker.check(graph, {"trust"});

    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "orphaned_node:orphan") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "unresolved_choice:impossible") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "impossible_condition:impossible:trust") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "missing_localization_key:orphan") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "missing_ending") != diagnostics.end());
}
