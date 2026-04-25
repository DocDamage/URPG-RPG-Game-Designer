#include "engine/core/dialogue/dialogue_graph.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dialogue graph supports speaker metadata localization choices effects and preview", "[dialogue][narrative][ffs10]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({
        "start",
        "guide",
        "Guide",
        "dialogue.start",
        "Welcome.",
        false,
        {{"choice_help", "Help", "end", {{"guide_affinity", ">=", 0}}, {{"guide_affinity", 5}}}},
    }));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "dialogue.end", "Thanks.", true, {}}));

    const auto route = graph.previewRoute();
    const auto json = graph.serialize();

    REQUIRE(route == std::vector<std::string>{"start", "end"});
    REQUIRE(json["nodes"].size() == 2);
    const auto* start = graph.findNode("start");
    REQUIRE(start != nullptr);
    REQUIRE(start->choices[0].effects[0].key == "guide_affinity");
}
