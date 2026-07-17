#include "engine/core/playtest/playtest_event_debugger_runtime.h"

namespace urpg::playtest {

bool PlaytestEventDebuggerRuntime::start(std::string session_id,
                                         const events::EventDocument& document,
                                         const std::string& event_id,
                                         events::EventWorldState state,
                                         std::string* diagnostic) {
    active_ = false;
    revision_ = 0;
    session_id_.clear();
    if (session_id.empty() || event_id.empty()) {
        if (diagnostic) *diagnostic = "playtest_event_debugger_start_invalid";
        return false;
    }
    document_ = document;
    debugger_.start(document_, event_id, state);
    if (!debugger_.snapshot().running) {
        if (diagnostic) *diagnostic = "playtest_event_debugger_event_unavailable";
        return false;
    }
    session_id_ = std::move(session_id);
    revision_ = 1;
    active_ = publish(diagnostic);
    return active_;
}

bool PlaytestEventDebuggerRuntime::tick(std::string* diagnostic) {
    if (!active_) return false;
    const auto controls = bridge_.pollControls([&](const EventTraceControl& control) {
        return control.expected_trace_revision == revision_ &&
               applyEventTraceControl(debugger_, control);
    });
    if (controls.io_error) {
        if (diagnostic) *diagnostic = controls.error;
        return false;
    }
    if (controls.applied > 0) {
        ++revision_;
        return publish(diagnostic);
    }
    const auto before = debugger_.snapshot();
    if (!before.running || before.paused_manually || before.paused_on_breakpoint) return true;
    (void)debugger_.step();
    ++revision_;
    return publish(diagnostic);
}

bool PlaytestEventDebuggerRuntime::publish(std::string* diagnostic) {
    return bridge_.publish(session_id_, revision_, debugger_.snapshot(), diagnostic);
}

} // namespace urpg::playtest
