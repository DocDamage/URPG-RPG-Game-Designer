#include "editor/events/event_authoring_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EventAuthoringPanel renders document graph diagnostics and debugger summary", "[events][authoring][editor]") {
    using namespace urpg::events;
    urpg::editor::EventAuthoringPanel panel;

    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.addEvent(EventDefinition{
        "evt",
        "town",
        1,
        1,
        {EventPage{"p", 0, EventTrigger::ActionButton, {}, {
            EventCommand{"switch", EventCommandKind::Switch, "s1", "true"}
        }}}
    });

    panel.model().load(std::move(document));
    panel.model().startDebugging("evt");
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().event_count == 1);
    REQUIRE(panel.lastRenderSnapshot().page_count == 1);
    REQUIRE(panel.lastRenderSnapshot().command_count == 1);
    REQUIRE(panel.lastRenderSnapshot().dependency_edge_count == 1);
    REQUIRE(panel.lastRenderSnapshot().diagnostic_count == 0);
    REQUIRE(panel.lastRenderSnapshot().has_active_page);
    REQUIRE(panel.lastRenderSnapshot().debugger_running);
}

TEST_CASE("EventAuthoringPanel renders empty document snapshot without crashing", "[events][authoring][editor]") {
    urpg::editor::EventAuthoringPanel panel;

    panel.model().load(urpg::events::EventDocument{});
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().event_count == 0);
    REQUIRE(panel.lastRenderSnapshot().page_count == 0);
    REQUIRE(panel.lastRenderSnapshot().command_count == 0);
    REQUIRE(panel.lastRenderSnapshot().dependency_edge_count == 0);
    REQUIRE(panel.lastRenderSnapshot().diagnostic_count == 0);
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_active_page);
    REQUIRE_FALSE(panel.lastRenderSnapshot().debugger_running);
}
