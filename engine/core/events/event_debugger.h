#pragma once

#include "engine/core/events/event_document.h"
#include "engine/core/events/native_event_command_matrix.h"

#include <optional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::events {

struct EventDebugFrame {
    std::string event_id;
    std::string page_id;
    size_t command_index = 0;
};

struct EventDebugSource {
    std::string route;
    std::string event_id;
    std::string page_id;
    std::string command_id;
    size_t command_index = 0;
};

struct EventDebugMutation {
    std::string kind;
    std::string id;
    std::string before;
    std::string after;
};

struct EventRuntimeTraceEntry {
    uint64_t sequence = 0;
    EventDebugSource source;
    EventCommandKind command_kind = EventCommandKind::Unsupported;
    size_t stack_depth = 0;
    std::string lane_id = "main";
    std::optional<bool> branch_taken;
    int64_t wait_frames = 0;
    std::vector<EventDebugMutation> mutations;
    std::string result_code;
};

enum class EventBreakpointConditionKind : uint8_t {
    Always,
    VariableEquals,
    VariableAtLeast,
    SwitchEquals,
    SelfSwitchEquals
};

struct EventBreakpointCondition {
    EventBreakpointConditionKind kind = EventBreakpointConditionKind::Always;
    std::string target;
    int64_t number_value = 0;
    bool bool_value = false;
};

struct EventBreakpoint {
    std::string id;
    std::string event_id;
    std::string page_id;
    size_t command_index = 0;
    EventBreakpointCondition condition;
    bool enabled = true;
    uint64_t hit_count = 0;
};

struct EventDebugSnapshot {
    bool running = false;
    bool paused_on_breakpoint = false;
    bool paused_manually = false;
    bool frame_advance_available = false;
    uint64_t runtime_frame = 0;
    std::optional<std::string> active_breakpoint_id;
    std::optional<EventDebugFrame> current_frame;
    std::optional<EventCommand> current_command;
    std::optional<EventDebugSource> current_source;
    std::vector<EventDebugFrame> stack;
    std::map<std::string, int64_t> watched_variables;
    size_t branch_depth = 0;
    int64_t wait_frames = 0;
    std::vector<std::string> parallel_lane_ids;
    std::vector<EventRuntimeTraceEntry> trace;
    std::vector<EventBreakpoint> breakpoints;
};

class EventDebugger {
public:
    void start(const EventDocument& document, const std::string& event_id, const EventWorldState& state);
    void addBreakpoint(std::string event_id, std::string page_id, size_t command_index);
    bool addConditionalBreakpoint(EventBreakpoint breakpoint);
    bool setBreakpointEnabled(const std::string& breakpoint_id, bool enabled);
    bool removeBreakpoint(const std::string& breakpoint_id);
    void pause();
    bool step();
    bool advanceFrame();
    void continueExecution();
    void resume();
    void watchVariable(std::string variable_id);
    std::optional<EventDebugSource> sourceForTrace(uint64_t sequence) const;
    std::optional<EventDebugSource> sourceForBreakpoint(const std::string& breakpoint_id) const;
    EventDebugSnapshot snapshot() const;

private:
    static std::string breakpointKey(const std::string& event_id, const std::string& page_id, size_t command_index);
    const std::vector<EventCommand>* commandsForFrame(const EventDebugFrame& frame) const;
    static EventDebugSource sourceFor(const EventDebugFrame& frame, const EventCommand& command);
    bool conditionMatches(const EventBreakpointCondition& condition) const;
    EventBreakpoint* matchingBreakpoint(const EventDebugFrame& frame);

    const EventDocument* document_ = nullptr;
    EventWorldState state_;
    NativeEventRuntimeState native_state_;
    std::vector<EventDebugFrame> stack_;
    std::vector<EventRuntimeTraceEntry> trace_;
    std::vector<std::string> parallel_lane_ids_;
    std::map<std::string, EventBreakpoint> breakpoints_;
    std::set<std::string> watched_variables_;
    bool paused_on_breakpoint_ = false;
    bool paused_manually_ = false;
    std::optional<std::string> active_breakpoint_id_;
    uint64_t runtime_frame_ = 0;
    uint64_t next_trace_sequence_ = 1;
};

} // namespace urpg::events
