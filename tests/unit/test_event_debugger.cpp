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

TEST_CASE("EventDebugger traces quest failure through branches waits lanes common events and source",
          "[events][runtime_trace][pcq406]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap({"town", 20, 20});
    document.addCommonEvent({"quest_fail", {
        {"set_failed", EventCommandKind::Switch, "quest.failed", "true"},
        {"failure_message", EventCommandKind::Message, "", "The ritual failed."}
    }});
    document.addEvent({"ritual", "town", 3, 4, {{"main", 0, EventTrigger::ActionButton, {}, {
        {"quest_stage", EventCommandKind::Variable, "quest.stage", "", 2},
        {"check_ready", EventCommandKind::Condition, "ritual.ready"},
        {"wait_fx", EventCommandKind::Wait, "", "", 12},
        {"ambient_lane", EventCommandKind::Parallel},
        {"fail", EventCommandKind::CommonEvent, "quest_fail"},
        {"end", EventCommandKind::EndBranch}
    }}}});

    EventDebugger debugger;
    debugger.start(document, "ritual", {});
    REQUIRE(debugger.snapshot().current_command->id == "quest_stage");
    REQUIRE(debugger.snapshot().current_source->route == "event_editor");
    debugger.resume();

    const auto snapshot = debugger.snapshot();
    REQUIRE_FALSE(snapshot.running);
    REQUIRE(snapshot.trace.size() == 8);
    REQUIRE(snapshot.trace[0].mutations[0].id == "quest.stage");
    REQUIRE(snapshot.trace[0].mutations[0].before == "0");
    REQUIRE(snapshot.trace[0].mutations[0].after == "2");
    REQUIRE(snapshot.trace[1].branch_taken == false);
    REQUIRE(snapshot.trace[2].wait_frames == 12);
    REQUIRE(snapshot.trace[3].lane_id == "parallel:ambient_lane");
    REQUIRE(snapshot.parallel_lane_ids == std::vector<std::string>{"parallel:ambient_lane"});
    REQUIRE(snapshot.trace[5].source.route == "common_event_editor");
    REQUIRE(snapshot.trace[5].source.event_id == "common:quest_fail");
    REQUIRE(snapshot.trace[5].mutations[0].id == "quest.failed");
    REQUIRE(snapshot.trace[5].stack_depth == 2);
    REQUIRE(snapshot.trace.back().source.command_id == "end");

    const auto source = debugger.sourceForTrace(snapshot.trace[5].sequence);
    REQUIRE(source);
    REQUIRE(source->page_id == "quest_fail");
    REQUIRE_FALSE(debugger.sourceForTrace(999999));
}
