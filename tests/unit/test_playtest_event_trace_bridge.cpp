#include "editor/diagnostics/diagnostics_workspace.h"
#include "engine/core/playtest/playtest_event_trace_bridge.h"
#include "engine/core/playtest/playtest_event_debugger_runtime.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

std::filesystem::path uniqueSession() {
    return std::filesystem::temp_directory_path() /
           ("urpg_event_trace_bridge_" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

urpg::events::EventDebugSnapshot traceSnapshot() {
    urpg::events::EventDebugSnapshot snapshot;
    snapshot.running = true;
    snapshot.paused_manually = true;
    snapshot.paused_on_breakpoint = true;
    snapshot.frame_advance_available = true;
    snapshot.runtime_frame = 42;
    snapshot.active_breakpoint_id = "break.ritual";
    snapshot.current_source = {"event_editor", "ritual", "main", "ambient", 1};
    snapshot.stack = {{"ritual", "main", 1}, {"common:quest_fail", "quest_fail", 0}};
    snapshot.breakpoints = {{"break.ritual", "ritual", "main", 1,
                             {urpg::events::EventBreakpointConditionKind::VariableAtLeast,
                              "quest.stage", 1, false}, true, 2}};
    snapshot.watched_variables = {{"quest.stage", 1}};
    snapshot.parallel_lane_ids = {"parallel:ambient"};
    snapshot.trace = {
        {1, {"event_editor", "ritual", "main", "start", 0},
         urpg::events::EventCommandKind::Variable, 1, "main", {}, 0,
         {{"variable", "quest.stage", "0", "1"}}, "event_command_applied"},
        {2, {"event_editor", "ritual", "main", "ambient", 1},
         urpg::events::EventCommandKind::Parallel, 1, "parallel:ambient", {}, 0, {},
         "parallel_lane_started"},
        {3, {"common_event_editor", "common:quest_fail", "quest_fail", "message", 0},
         urpg::events::EventCommandKind::Message, 2, "main", {}, 0, {},
         "event_command_applied"},
    };
    return snapshot;
}

} // namespace

TEST_CASE("live event trace crosses the process bridge and rejects stale or malformed snapshots",
          "[playtest][events][runtime_trace][pcq406]") {
    const auto session = uniqueSession();
    std::filesystem::remove_all(session);
    urpg::playtest::PlaytestEventTraceBridge runtime(session);
    urpg::playtest::PlaytestEventTraceBridge editor(session);

    REQUIRE(runtime.publish("session-001", 7, traceSnapshot()));
    const auto live = editor.readAfter(0);
    REQUIRE(live.has_value());
    REQUIRE(live->revision == 7);
    REQUIRE(live->paused);
    REQUIRE(live->paused_on_breakpoint);
    REQUIRE(live->frame_advance_available);
    REQUIRE(live->active_breakpoint_id == "break.ritual");
    REQUIRE(live->current_source->command_id == "ambient");
    REQUIRE(live->stack.size() == 2);
    REQUIRE(live->breakpoints.size() == 1);
    REQUIRE(live->breakpoints[0].condition.kind ==
            urpg::events::EventBreakpointConditionKind::VariableAtLeast);
    REQUIRE(live->watched_variables.at("quest.stage") == 1);
    REQUIRE(live->trace.size() == 3);
    REQUIRE(live->trace[1].lane_id == "parallel:ambient");
    REQUIRE(live->trace[2].source.route == "common_event_editor");
    REQUIRE_FALSE(editor.readAfter(7).has_value());

    urpg::playtest::EventTraceControl resume;
    resume.control_id = 1;
    resume.action = urpg::playtest::EventTraceControlAction::Continue;
    resume.expected_trace_revision = 7;
    REQUIRE(editor.appendControl(resume));
    auto stale = resume;
    stale.control_id = 2;
    stale.action = urpg::playtest::EventTraceControlAction::Step;
    stale.expected_trace_revision = 6;
    REQUIRE(editor.appendControl(stale));
    const auto controls = runtime.pollControls([](const auto& control) {
        return control.expected_trace_revision == 7;
    });
    REQUIRE(controls.processed == 2);
    REQUIRE(controls.applied == 1);
    REQUIRE(controls.rejected == 1);

    std::filesystem::remove_all(session);
}

TEST_CASE("production diagnostics route filters lanes controls pause state and opens exact event source",
          "[editor][diagnostics][events][runtime_trace][pcq406]") {
    const auto session = uniqueSession();
    std::filesystem::remove_all(session);
    urpg::playtest::PlaytestEventTraceBridge runtime(session);
    urpg::playtest::PlaytestEventTraceBridge editor(session);
    REQUIRE(runtime.publish("session-002", 11, traceSnapshot()));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindEventTraceBridge(&editor);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventTrace);
    urpg::events::EventDebugSource opened;
    workspace.eventTracePanel().setSourceOpenHandler([&](const auto& source) {
        opened = source;
        return true;
    });
    workspace.render();

    auto summary = workspace.tabSummary(urpg::editor::DiagnosticsTab::EventTrace);
    REQUIRE(summary.has_data);
    REQUIRE(summary.item_count == 3);
    REQUIRE(workspace.eventTracePanel().snapshot().paused);
    REQUIRE(workspace.eventTracePanel().snapshot().paused_on_breakpoint);
    REQUIRE(workspace.eventTracePanel().snapshot().call_stack.size() == 2);
    REQUIRE(workspace.eventTracePanel().snapshot().breakpoints.size() == 1);
    REQUIRE(workspace.eventTracePanel().snapshot().parallel_lane_ids ==
            std::vector<std::string>{"parallel:ambient"});

    REQUIRE(workspace.eventTracePanel().setLaneFilter("parallel:ambient"));
    REQUIRE(workspace.eventTracePanel().visibleRows().size() == 1);
    REQUIRE(workspace.eventTracePanel().selectVisibleRow(0));
    REQUIRE(workspace.eventTracePanel().openSelectedSource());
    REQUIRE(opened.command_id == "ambient");
    REQUIRE(opened.route == "event_editor");
    REQUIRE(workspace.eventTracePanel().openCurrentSource());
    REQUIRE(opened.command_id == "ambient");
    REQUIRE(workspace.eventTracePanel().requestAddBreakpoint(
        {"break.new", "ritual", "main", 2,
         {urpg::events::EventBreakpointConditionKind::VariableEquals, "quest.stage", 2, false}, true, 0}));
    REQUIRE(workspace.eventTracePanel().requestBreakpointEnabled("break.ritual", false));
    REQUIRE(workspace.eventTracePanel().requestWatchVariable("quest.result"));
    REQUIRE(workspace.eventTracePanel().requestControl(
        urpg::playtest::EventTraceControlAction::Continue));
    REQUIRE(workspace.eventTracePanel().snapshot().last_control_code ==
            "event_trace_control_queued");

    const auto controls = runtime.pollControls([](const auto& control) {
        if (control.expected_trace_revision != 11) return false;
        if (control.action == urpg::playtest::EventTraceControlAction::DisableBreakpoint) {
            return control.breakpoint_id == "break.ritual";
        }
        if (control.action == urpg::playtest::EventTraceControlAction::AddBreakpoint) {
            return control.breakpoint.id == "break.new" &&
                   control.breakpoint.condition.kind ==
                       urpg::events::EventBreakpointConditionKind::VariableEquals;
        }
        if (control.action == urpg::playtest::EventTraceControlAction::WatchVariable) {
            return control.variable_id == "quest.result";
        }
        return control.action == urpg::playtest::EventTraceControlAction::Continue;
    });
    REQUIRE(controls.applied == 4);

    std::filesystem::remove_all(session);
}

TEST_CASE("live debugger breakpoint controls apply to the native EventDebugger",
          "[playtest][events][debugger][pcq503]") {
    urpg::events::EventDebugger debugger;
    urpg::playtest::EventTraceControl add;
    add.action = urpg::playtest::EventTraceControlAction::AddBreakpoint;
    add.breakpoint = {"break.conditional", "ritual", "main", 4,
                      {urpg::events::EventBreakpointConditionKind::SwitchEquals,
                       "gate.open", 0, true}, true, 0};
    REQUIRE(urpg::playtest::applyEventTraceControl(debugger, add));
    REQUIRE(debugger.snapshot().breakpoints.size() == 1);
    REQUIRE(debugger.snapshot().breakpoints[0].condition.target == "gate.open");

    urpg::playtest::EventTraceControl disable;
    disable.action = urpg::playtest::EventTraceControlAction::DisableBreakpoint;
    disable.breakpoint_id = "break.conditional";
    REQUIRE(urpg::playtest::applyEventTraceControl(debugger, disable));
    REQUIRE_FALSE(debugger.snapshot().breakpoints[0].enabled);

    urpg::playtest::EventTraceControl watch;
    watch.action = urpg::playtest::EventTraceControlAction::WatchVariable;
    watch.variable_id = "quest.stage";
    REQUIRE(urpg::playtest::applyEventTraceControl(debugger, watch));
    REQUIRE(debugger.snapshot().watched_variables.contains("quest.stage"));

    urpg::playtest::EventTraceControl remove;
    remove.action = urpg::playtest::EventTraceControlAction::RemoveBreakpoint;
    remove.breakpoint_id = "break.conditional";
    REQUIRE(urpg::playtest::applyEventTraceControl(debugger, remove));
    REQUIRE(debugger.snapshot().breakpoints.empty());
}

TEST_CASE("playtest runtime debugger lane publishes breakpoints and exact source across frames",
          "[playtest][events][debugger][pcq503][runtime_bridge]") {
    const auto session = uniqueSession();
    std::filesystem::remove_all(session);
    urpg::events::EventDocument document;
    document.addMap({"town", 8, 8});
    document.addEvent({"ritual", "town", 1, 1, {{"main", 0,
        urpg::events::EventTrigger::ActionButton, {}, {
            {"set_stage", urpg::events::EventCommandKind::Variable, "quest.stage", {}, 2},
            {"show_result", urpg::events::EventCommandKind::Message, {}, "Done"},
        }}}});

    urpg::playtest::PlaytestEventDebuggerRuntime runtime(session);
    urpg::playtest::PlaytestEventTraceBridge editor(session);
    REQUIRE(runtime.start("session-debugger", document, "ritual"));
    const auto initial = editor.readAfter(0);
    REQUIRE(initial);
    REQUIRE(initial->revision == 1);
    REQUIRE(initial->current_source->command_id == "set_stage");

    urpg::playtest::EventTraceControl add;
    add.control_id = 1;
    add.action = urpg::playtest::EventTraceControlAction::AddBreakpoint;
    add.expected_trace_revision = 1;
    add.breakpoint = {"break.stage", "ritual", "main", 0, {}, true, 0};
    REQUIRE(editor.appendControl(add));
    REQUIRE(runtime.tick());
    const auto configured = editor.readAfter(1);
    REQUIRE(configured);
    REQUIRE(configured->breakpoints.size() == 1);

    REQUIRE(runtime.tick());
    const auto paused = editor.readAfter(2);
    REQUIRE(paused);
    REQUIRE(paused->paused_on_breakpoint);
    REQUIRE(paused->active_breakpoint_id == "break.stage");
    REQUIRE(paused->current_source->command_id == "set_stage");

    urpg::playtest::EventTraceControl step;
    step.control_id = 2;
    step.action = urpg::playtest::EventTraceControlAction::Step;
    step.expected_trace_revision = 3;
    REQUIRE(editor.appendControl(step));
    REQUIRE(runtime.tick());
    const auto stepped = editor.readAfter(3);
    REQUIRE(stepped);
    REQUIRE(stepped->trace.size() == 1);
    REQUIRE(stepped->trace.front().source.command_id == "set_stage");
    REQUIRE(stepped->watched_variables.empty());
    std::filesystem::remove_all(session);
}
