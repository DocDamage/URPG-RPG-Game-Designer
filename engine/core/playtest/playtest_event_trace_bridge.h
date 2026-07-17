#pragma once

#include "engine/core/events/event_debugger.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace urpg::playtest {

struct LiveEventTraceSnapshot {
    uint64_t revision = 0;
    std::string session_id;
    uint64_t runtime_frame = 0;
    bool running = false;
    bool paused = false;
    bool paused_on_breakpoint = false;
    bool frame_advance_available = false;
    std::optional<std::string> active_breakpoint_id;
    std::optional<events::EventDebugSource> current_source;
    std::vector<events::EventDebugFrame> stack;
    std::vector<events::EventBreakpoint> breakpoints;
    std::map<std::string, int64_t> watched_variables;
    std::vector<std::string> parallel_lane_ids;
    std::vector<events::EventRuntimeTraceEntry> trace;
};

enum class EventTraceControlAction : uint8_t {
    Pause,
    Continue,
    Step,
    AdvanceFrame,
    AddBreakpoint,
    RemoveBreakpoint,
    EnableBreakpoint,
    DisableBreakpoint,
    WatchVariable
};

struct EventTraceControl {
    uint64_t control_id = 0;
    EventTraceControlAction action = EventTraceControlAction::Pause;
    uint64_t expected_trace_revision = 0;
    events::EventBreakpoint breakpoint;
    std::string breakpoint_id;
    std::string variable_id;
};

bool applyEventTraceControl(events::EventDebugger& debugger, const EventTraceControl& control);

struct EventTraceControlPollResult {
    size_t processed = 0;
    size_t applied = 0;
    size_t rejected = 0;
    bool io_error = false;
    std::string error;
};

// File-backed, bounded process bridge. The runtime atomically publishes one
// latest trace snapshot while the editor appends revision-guarded controls.
class PlaytestEventTraceBridge {
public:
    using ControlHandler = std::function<bool(const EventTraceControl&)>;

    explicit PlaytestEventTraceBridge(std::filesystem::path session_directory)
        : session_directory_(std::move(session_directory)) {}

    bool publish(std::string session_id, uint64_t revision,
                 const events::EventDebugSnapshot& snapshot, std::string* diagnostic = nullptr) const;
    std::optional<LiveEventTraceSnapshot> readAfter(uint64_t revision,
                                                   std::string* diagnostic = nullptr) const;
    bool appendControl(const EventTraceControl& control, std::string* diagnostic = nullptr) const;
    EventTraceControlPollResult pollControls(const ControlHandler& handler, size_t max_controls = 8);

private:
    std::filesystem::path session_directory_;
    uintmax_t consumed_control_bytes_ = 0;
};

} // namespace urpg::playtest
