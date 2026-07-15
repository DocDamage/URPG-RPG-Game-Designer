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

TEST_CASE("dialogue choices preserve optional localization references", "[dialogue][localization]") {
    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"start", "guide", "Guide", "dialogue.start", "Welcome.", false, {}}));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "dialogue.end", "Thanks.", true, {}}));
    REQUIRE(graph.addChoice("start", {"choice_help", "Help", "end", {}, {}, "dialogue.choice.help"}));

    const auto serialized = graph.serialize();
    const auto restored = urpg::dialogue::DialogueGraph::fromJson(serialized);
    REQUIRE(restored.has_value());
    const auto* start = restored->findNode("start");
    REQUIRE(start != nullptr);
    REQUIRE(start->choices[0].localization_key == "dialogue.choice.help");

    urpg::editor::DialogueGraphPanel panel;
    panel.setGraph(*restored);
    REQUIRE(panel.beginInteractivePreview());
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["interactive_preview"]["choices"][0]["localization_key"] ==
            "dialogue.choice.help");

    const auto missing = restored->validateLocalizationKeys({"dialogue.start", "dialogue.end"});
    REQUIRE(missing.size() == 1);
    CHECK(missing[0].code == "missing_choice_localization_key");
    CHECK(missing[0].node_id == "start");
    CHECK(missing[0].choice_id == "choice_help");
    REQUIRE(restored->validateLocalizationKeys(
                {"dialogue.start", "dialogue.end", "dialogue.choice.help"})
                .empty());

    auto legacy = serialized;
    for (auto& node : legacy["nodes"]) {
        if (node["id"] == "start") {
            node["choices"][0].erase("localization_key");
        }
    }
    const auto legacy_restored = urpg::dialogue::DialogueGraph::fromJson(legacy);
    REQUIRE(legacy_restored.has_value());
    const auto* legacy_start = legacy_restored->findNode("start");
    REQUIRE(legacy_start != nullptr);
    REQUIRE(legacy_start->choices[0].localization_key.empty());
}
