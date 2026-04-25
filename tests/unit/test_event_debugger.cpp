#include "engine/core/events/event_debugger.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EventDebugger steps breaks resumes and exposes watch variables", "[events][authoring][debugger]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.addEvent(EventDefinition{
        "evt",
        "town",
        1,
        1,
        {EventPage{"p", 0, EventTrigger::ActionButton, {}, {
            EventCommand{"set_v", EventCommandKind::Variable, "score", {}, 7},
            EventCommand{"msg", EventCommandKind::Message, {}, "done"}
        }}}
    });

    EventDebugger debugger;
    debugger.addBreakpoint("evt", "p", 0);
    debugger.watchVariable("score");
    debugger.start(document, "evt", {});

    REQUIRE(debugger.step());
    REQUIRE(debugger.snapshot().paused_on_breakpoint);
    REQUIRE(debugger.step());
    REQUIRE(debugger.snapshot().watched_variables.at("score") == 7);

    debugger.resume();
    REQUIRE_FALSE(debugger.snapshot().running);
}
