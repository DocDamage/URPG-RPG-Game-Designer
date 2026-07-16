#include "engine/core/events/event_debugger.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::events::EventDocument makeDebugDocument() {
    using namespace urpg::events;
    EventDocument document;
    document.addMap({"town", 10, 10});
    document.addCommonEvent({"failure", {
        {"mark_failed", EventCommandKind::SelfSwitch, "A", "true"},
        {"failure_text", EventCommandKind::Message, {}, "failed"}
    }});
    document.addEvent({"evt", "town", 1, 1, {{"main", 0, EventTrigger::ActionButton, {}, {
        {"set_score", EventCommandKind::Variable, "score", {}, 5},
        {"wait", EventCommandKind::Wait, {}, {}, 2},
        {"call_failure", EventCommandKind::CommonEvent, "failure"},
        {"done", EventCommandKind::Message, {}, "done"}
    }}}});
    return document;
}

} // namespace

TEST_CASE("EventDebugger conditionally breaks and focuses the exact source", "[events][debugger][pcq503]") {
    using namespace urpg::events;
    const auto document = makeDebugDocument();
    EventDebugger debugger;
    REQUIRE(debugger.addConditionalBreakpoint({
        "score-ready", "evt", "main", 1,
        {EventBreakpointConditionKind::VariableAtLeast, "score", 5, false}}));
    REQUIRE_FALSE(debugger.addConditionalBreakpoint({}));
    debugger.start(document, "evt", {});

    REQUIRE(debugger.step());
    REQUIRE(debugger.step());
    const auto paused = debugger.snapshot();
    REQUIRE(paused.paused_on_breakpoint);
    REQUIRE(paused.active_breakpoint_id == "score-ready");
    REQUIRE(paused.current_command->id == "wait");
    REQUIRE(paused.stack.size() == 1);
    REQUIRE(paused.breakpoints[0].hit_count == 1);

    const auto source = debugger.sourceForBreakpoint("score-ready");
    REQUIRE(source);
    REQUIRE(source->route == "event_editor");
    REQUIRE(source->command_id == "wait");
    REQUIRE_FALSE(debugger.sourceForBreakpoint("missing"));
}

TEST_CASE("EventDebugger supports pause step continue and deterministic frame advance", "[events][debugger][pcq503]") {
    using namespace urpg::events;
    const auto document = makeDebugDocument();
    EventDebugger debugger;
    debugger.start(document, "evt", {});
    debugger.pause();
    REQUIRE(debugger.snapshot().paused_manually);

    REQUIRE(debugger.step());
    REQUIRE(debugger.snapshot().paused_manually);
    REQUIRE(debugger.step());
    REQUIRE(debugger.snapshot().wait_frames == 2);
    REQUIRE(debugger.snapshot().frame_advance_available);
    REQUIRE(debugger.advanceFrame());
    REQUIRE(debugger.advanceFrame());
    REQUIRE_FALSE(debugger.advanceFrame());
    REQUIRE(debugger.snapshot().runtime_frame == 2);

    debugger.continueExecution();
    const auto completed = debugger.snapshot();
    REQUIRE_FALSE(completed.running);
    REQUIRE_FALSE(completed.paused_manually);
    REQUIRE(completed.trace.size() == 6);
    REQUIRE(completed.trace[2].source.command_id == "call_failure");
    REQUIRE(completed.trace[3].source.route == "common_event_editor");
    REQUIRE(completed.trace[3].stack_depth == 2);
    REQUIRE(completed.trace.back().source.command_id == "done");
}

TEST_CASE("EventDebugger breakpoint lifecycle and self-switch conditions are deterministic", "[events][debugger][pcq503]") {
    using namespace urpg::events;
    const auto document = makeDebugDocument();
    EventDebugger debugger;
    REQUIRE(debugger.addConditionalBreakpoint({
        "self-switch", "common:failure", "failure", 1,
        {EventBreakpointConditionKind::SelfSwitchEquals, "A", 0, true}}));
    REQUIRE(debugger.setBreakpointEnabled("self-switch", false));
    REQUIRE_FALSE(debugger.setBreakpointEnabled("missing", true));
    REQUIRE(debugger.setBreakpointEnabled("self-switch", true));
    debugger.start(document, "evt", {});
    debugger.continueExecution();

    const auto paused = debugger.snapshot();
    REQUIRE(paused.running);
    REQUIRE(paused.paused_on_breakpoint);
    REQUIRE(paused.current_source->route == "common_event_editor");
    REQUIRE(paused.current_source->command_id == "failure_text");
    REQUIRE(paused.stack.size() == 2);
    REQUIRE(debugger.removeBreakpoint("self-switch"));
    REQUIRE_FALSE(debugger.removeBreakpoint("self-switch"));
    debugger.continueExecution();
    REQUIRE_FALSE(debugger.snapshot().running);
}
