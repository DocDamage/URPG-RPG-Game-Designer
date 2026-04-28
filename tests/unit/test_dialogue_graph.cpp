#include "engine/core/dialogue/dialogue_graph.h"
#include "editor/dialogue/dialogue_graph_panel.h"

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

    urpg::editor::DialogueGraphPanel panel;
    panel.setGraph(graph);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["node_count"] == 2);
    REQUIRE(panel.lastRenderSnapshot()["choice_count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["ending_count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["has_start_node"] == true);
    REQUIRE(panel.lastRenderSnapshot()["route_coverage"] == 1.0f);
    REQUIRE(panel.lastRenderSnapshot()["ux_focus_lane"] == "route_preview");
}
