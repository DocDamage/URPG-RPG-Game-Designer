#include "engine/core/events/event_dependency_graph.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("EventDependencyGraph reports read write and call edges", "[events][authoring][graph]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.addEvent(EventDefinition{
        "evt",
        "town",
        1,
        1,
        {EventPage{
            "p",
            0,
            EventTrigger::ActionButton,
            EventCondition{{{"s1", true}}, {{"v1", 3}}, {"quest.open"}},
            {
                EventCommand{"set_s2", EventCommandKind::Switch, "s2", "true"},
                EventCommand{"set_v2", EventCommandKind::Variable, "v2", {}, 9},
                EventCommand{"save", EventCommandKind::Message, {}, {}, 0, {"party.gold"}, {"quest.flags.intro"}},
                EventCommand{"call", EventCommandKind::CommonEvent, "ce", {}}
            }
        }}
    });

    const auto graph = EventDependencyGraph::build(document);
    const auto edges = graph.edgesForSource("evt/p");

    REQUIRE(edges.size() == 8);
    REQUIRE(std::any_of(edges.begin(), edges.end(), [](const auto& edge) {
        return edge.target_type == "switch" && edge.target_id == "s1" && edge.access == EventDependencyAccess::Read;
    }));
    REQUIRE(std::any_of(edges.begin(), edges.end(), [](const auto& edge) {
        return edge.target_type == "variable" && edge.target_id == "v2" && edge.access == EventDependencyAccess::Write;
    }));
    REQUIRE(std::any_of(edges.begin(), edges.end(), [](const auto& edge) {
        return edge.target_type == "common_event" && edge.target_id == "ce" && edge.access == EventDependencyAccess::Call;
    }));
    REQUIRE(std::any_of(edges.begin(), edges.end(), [](const auto& edge) {
        return edge.target_type == "save_field" && edge.target_id == "party.gold" && edge.access == EventDependencyAccess::Read;
    }));
    REQUIRE(std::any_of(edges.begin(), edges.end(), [](const auto& edge) {
        return edge.target_type == "save_field" && edge.target_id == "quest.flags.intro" && edge.access == EventDependencyAccess::Write;
    }));
}
