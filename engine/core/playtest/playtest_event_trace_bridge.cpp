#include "engine/core/playtest/playtest_event_trace_bridge.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace urpg::playtest {
namespace {

constexpr std::string_view kSnapshotSchema = "urpg.playtest_event_trace_snapshot.v2";
constexpr std::string_view kControlSchema = "urpg.playtest_event_trace_control.v2";
constexpr size_t kMaxTraceRows = 4096;
constexpr size_t kMaxBreakpoints = 512;
constexpr size_t kMaxStackFrames = 128;

std::string actionName(const EventTraceControlAction action) {
    switch (action) {
    case EventTraceControlAction::Pause: return "pause";
    case EventTraceControlAction::Continue: return "continue";
    case EventTraceControlAction::Step: return "step";
    case EventTraceControlAction::AdvanceFrame: return "advance_frame";
    case EventTraceControlAction::AddBreakpoint: return "add_breakpoint";
    case EventTraceControlAction::RemoveBreakpoint: return "remove_breakpoint";
    case EventTraceControlAction::EnableBreakpoint: return "enable_breakpoint";
    case EventTraceControlAction::DisableBreakpoint: return "disable_breakpoint";
    case EventTraceControlAction::WatchVariable: return "watch_variable";
    }
    return "pause";
}

std::optional<EventTraceControlAction> parseAction(const std::string& value) {
    if (value == "pause") return EventTraceControlAction::Pause;
    if (value == "continue") return EventTraceControlAction::Continue;
    if (value == "step") return EventTraceControlAction::Step;
    if (value == "advance_frame") return EventTraceControlAction::AdvanceFrame;
    if (value == "add_breakpoint") return EventTraceControlAction::AddBreakpoint;
    if (value == "remove_breakpoint") return EventTraceControlAction::RemoveBreakpoint;
    if (value == "enable_breakpoint") return EventTraceControlAction::EnableBreakpoint;
    if (value == "disable_breakpoint") return EventTraceControlAction::DisableBreakpoint;
    if (value == "watch_variable") return EventTraceControlAction::WatchVariable;
    return std::nullopt;
}

std::string conditionName(const events::EventBreakpointConditionKind kind) {
    switch (kind) {
    case events::EventBreakpointConditionKind::Always: return "always";
    case events::EventBreakpointConditionKind::VariableEquals: return "variable_equals";
    case events::EventBreakpointConditionKind::VariableAtLeast: return "variable_at_least";
    case events::EventBreakpointConditionKind::SwitchEquals: return "switch_equals";
    case events::EventBreakpointConditionKind::SelfSwitchEquals: return "self_switch_equals";
    }
    return "always";
}

std::optional<events::EventBreakpointConditionKind> parseCondition(const std::string& value) {
    if (value == "always") return events::EventBreakpointConditionKind::Always;
    if (value == "variable_equals") return events::EventBreakpointConditionKind::VariableEquals;
    if (value == "variable_at_least") return events::EventBreakpointConditionKind::VariableAtLeast;
    if (value == "switch_equals") return events::EventBreakpointConditionKind::SwitchEquals;
    if (value == "self_switch_equals") return events::EventBreakpointConditionKind::SelfSwitchEquals;
    return std::nullopt;
}

nlohmann::json sourceJson(const events::EventDebugSource& source) {
    return {{"route", source.route}, {"eventId", source.event_id}, {"pageId", source.page_id},
            {"commandId", source.command_id}, {"commandIndex", source.command_index}};
}

nlohmann::json frameJson(const events::EventDebugFrame& frame) {
    return {{"eventId", frame.event_id}, {"pageId", frame.page_id},
            {"commandIndex", frame.command_index}};
}

std::optional<events::EventDebugFrame> parseFrame(const nlohmann::json& value) {
    if (!value.is_object()) return std::nullopt;
    events::EventDebugFrame frame{value.value("eventId", ""), value.value("pageId", ""),
                                  value.value("commandIndex", size_t{0})};
    if (frame.event_id.empty() || frame.page_id.empty()) return std::nullopt;
    return frame;
}

std::optional<events::EventDebugSource> parseSource(const nlohmann::json& value) {
    if (!value.is_object()) return std::nullopt;
    events::EventDebugSource source{value.value("route", ""), value.value("eventId", ""),
                                    value.value("pageId", ""), value.value("commandId", ""),
                                    value.value("commandIndex", size_t{0})};
    if (source.route.empty() || source.event_id.empty() || source.page_id.empty() || source.command_id.empty()) {
        return std::nullopt;
    }
    return source;
}

nlohmann::json breakpointJson(const events::EventBreakpoint& breakpoint) {
    return {{"id", breakpoint.id}, {"eventId", breakpoint.event_id}, {"pageId", breakpoint.page_id},
            {"commandIndex", breakpoint.command_index}, {"enabled", breakpoint.enabled},
            {"hitCount", breakpoint.hit_count},
            {"condition", {{"kind", conditionName(breakpoint.condition.kind)},
                            {"target", breakpoint.condition.target},
                            {"numberValue", breakpoint.condition.number_value},
                            {"boolValue", breakpoint.condition.bool_value}}}};
}

std::optional<events::EventBreakpoint> parseBreakpoint(const nlohmann::json& value) {
    if (!value.is_object() || !value.contains("condition") || !value["condition"].is_object()) return std::nullopt;
    const auto& conditionValue = value["condition"];
    const auto kind = parseCondition(conditionValue.value("kind", ""));
    if (!kind) return std::nullopt;
    events::EventBreakpoint breakpoint;
    breakpoint.id = value.value("id", "");
    breakpoint.event_id = value.value("eventId", "");
    breakpoint.page_id = value.value("pageId", "");
    breakpoint.command_index = value.value("commandIndex", size_t{0});
    breakpoint.enabled = value.value("enabled", true);
    breakpoint.hit_count = value.value("hitCount", uint64_t{0});
    breakpoint.condition = {*kind, conditionValue.value("target", ""),
                            conditionValue.value("numberValue", int64_t{0}),
                            conditionValue.value("boolValue", false)};
    if (breakpoint.id.empty() || breakpoint.event_id.empty() || breakpoint.page_id.empty()) return std::nullopt;
    return breakpoint;
}

nlohmann::json traceJson(const events::EventRuntimeTraceEntry& row) {
    nlohmann::json mutations = nlohmann::json::array();
    for (const auto& mutation : row.mutations) {
        mutations.push_back({{"kind", mutation.kind}, {"id", mutation.id},
                             {"before", mutation.before}, {"after", mutation.after}});
    }
    nlohmann::json value{{"sequence", row.sequence},
                         {"source", sourceJson(row.source)},
                         {"commandKind", toString(row.command_kind)},
                         {"stackDepth", row.stack_depth},
                         {"laneId", row.lane_id},
                         {"waitFrames", row.wait_frames},
                         {"mutations", std::move(mutations)},
                         {"resultCode", row.result_code}};
    if (row.branch_taken.has_value()) value["branchTaken"] = *row.branch_taken;
    return value;
}

std::optional<events::EventRuntimeTraceEntry> parseTrace(const nlohmann::json& value) {
    if (!value.is_object() || !value.contains("source") || !value["source"].is_object()) return std::nullopt;
    const auto& source = value["source"];
    events::EventRuntimeTraceEntry row;
    row.sequence = value.value("sequence", uint64_t{0});
    row.source.route = source.value("route", "");
    row.source.event_id = source.value("eventId", "");
    row.source.page_id = source.value("pageId", "");
    row.source.command_id = source.value("commandId", "");
    row.source.command_index = source.value("commandIndex", size_t{0});
    row.command_kind = events::eventCommandKindFromString(value.value("commandKind", "unsupported"));
    row.stack_depth = value.value("stackDepth", size_t{0});
    row.lane_id = value.value("laneId", "main");
    if (value.contains("branchTaken") && value["branchTaken"].is_boolean()) {
        row.branch_taken = value["branchTaken"].get<bool>();
    }
    row.wait_frames = value.value("waitFrames", int64_t{0});
    row.result_code = value.value("resultCode", "");
    for (const auto& item : value.value("mutations", nlohmann::json::array())) {
        if (!item.is_object()) return std::nullopt;
        row.mutations.push_back({item.value("kind", ""), item.value("id", ""),
                                 item.value("before", ""), item.value("after", "")});
    }
    if (row.sequence == 0 || row.source.route.empty() || row.source.command_id.empty() || row.lane_id.empty()) {
        return std::nullopt;
    }
    return row;
}

bool atomicWrite(const std::filesystem::path& target, const nlohmann::json& value,
                 std::string* diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        if (diagnostic) *diagnostic = "event_trace_directory_failed:" + error.message();
        return false;
    }
    auto temporary = target;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        if (diagnostic) *diagnostic = "event_trace_temporary_open_failed";
        return false;
    }
    output << value.dump() << '\n';
    output.close();
    if (!output) {
        std::filesystem::remove(temporary, error);
        if (diagnostic) *diagnostic = "event_trace_temporary_flush_failed";
        return false;
    }
    auto backup = target;
    backup += ".bak";
    const bool replacing = std::filesystem::exists(target);
    if (replacing) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(target, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            if (diagnostic) *diagnostic = "event_trace_backup_failed:" + error.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, target, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, target, restore_error);
        }
        if (diagnostic) *diagnostic = "event_trace_publish_failed:" + error.message();
        return false;
    }
    if (replacing) std::filesystem::remove(backup, error);
    return true;
}

} // namespace

bool PlaytestEventTraceBridge::publish(std::string session_id, const uint64_t revision,
                                       const events::EventDebugSnapshot& snapshot,
                                       std::string* diagnostic) const {
    if (session_id.empty() || revision == 0 || snapshot.trace.size() > kMaxTraceRows ||
        snapshot.breakpoints.size() > kMaxBreakpoints || snapshot.stack.size() > kMaxStackFrames) {
        if (diagnostic) *diagnostic = "event_trace_snapshot_invalid";
        return false;
    }
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& row : snapshot.trace) rows.push_back(traceJson(row));
    nlohmann::json stack = nlohmann::json::array();
    for (const auto& frame : snapshot.stack) stack.push_back(frameJson(frame));
    nlohmann::json breakpoints = nlohmann::json::array();
    for (const auto& breakpoint : snapshot.breakpoints) breakpoints.push_back(breakpointJson(breakpoint));
    nlohmann::json value{{"schema", kSnapshotSchema},
                         {"revision", revision},
                         {"sessionId", std::move(session_id)},
                         {"runtimeFrame", snapshot.runtime_frame},
                         {"running", snapshot.running},
                         {"paused", snapshot.paused_manually || snapshot.paused_on_breakpoint},
                         {"pausedOnBreakpoint", snapshot.paused_on_breakpoint},
                         {"frameAdvanceAvailable", snapshot.frame_advance_available},
                         {"parallelLaneIds", snapshot.parallel_lane_ids},
                         {"stack", std::move(stack)},
                         {"breakpoints", std::move(breakpoints)},
                         {"watchedVariables", snapshot.watched_variables},
                         {"trace", std::move(rows)}};
    if (snapshot.active_breakpoint_id) value["activeBreakpointId"] = *snapshot.active_breakpoint_id;
    if (snapshot.current_source) value["currentSource"] = sourceJson(*snapshot.current_source);
    return atomicWrite(session_directory_ / "event_trace_snapshot.json", value, diagnostic);
}

std::optional<LiveEventTraceSnapshot> PlaytestEventTraceBridge::readAfter(
    const uint64_t revision, std::string* diagnostic) const {
    std::ifstream input(session_directory_ / "event_trace_snapshot.json", std::ios::binary);
    if (!input) return std::nullopt;
    const auto value = nlohmann::json::parse(input, nullptr, false);
    if (!value.is_object() || value.value("schema", "") != kSnapshotSchema) {
        if (diagnostic) *diagnostic = "event_trace_snapshot_malformed";
        return std::nullopt;
    }
    LiveEventTraceSnapshot result;
    result.revision = value.value("revision", uint64_t{0});
    result.session_id = value.value("sessionId", "");
    result.runtime_frame = value.value("runtimeFrame", uint64_t{0});
    result.running = value.value("running", false);
    result.paused = value.value("paused", false);
    result.paused_on_breakpoint = value.value("pausedOnBreakpoint", false);
    result.frame_advance_available = value.value("frameAdvanceAvailable", false);
    if (value.contains("activeBreakpointId") && value["activeBreakpointId"].is_string()) {
        result.active_breakpoint_id = value["activeBreakpointId"].get<std::string>();
    }
    if (value.contains("currentSource")) {
        result.current_source = parseSource(value["currentSource"]);
        if (!result.current_source) {
            if (diagnostic) *diagnostic = "event_trace_current_source_invalid";
            return std::nullopt;
        }
    }
    result.parallel_lane_ids = value.value("parallelLaneIds", std::vector<std::string>{});
    result.watched_variables = value.value("watchedVariables", std::map<std::string, int64_t>{});
    const auto& rows = value.value("trace", nlohmann::json::array());
    const auto& stack = value.value("stack", nlohmann::json::array());
    const auto& breakpoints = value.value("breakpoints", nlohmann::json::array());
    if (result.revision == 0 || result.session_id.empty() || rows.size() > kMaxTraceRows ||
        stack.size() > kMaxStackFrames || breakpoints.size() > kMaxBreakpoints) {
        if (diagnostic) *diagnostic = "event_trace_snapshot_invalid";
        return std::nullopt;
    }
    if (result.revision <= revision) return std::nullopt;
    for (const auto& item : stack) {
        auto frame = parseFrame(item);
        if (!frame) {
            if (diagnostic) *diagnostic = "event_trace_stack_frame_invalid";
            return std::nullopt;
        }
        result.stack.push_back(std::move(*frame));
    }
    for (const auto& item : breakpoints) {
        auto breakpoint = parseBreakpoint(item);
        if (!breakpoint) {
            if (diagnostic) *diagnostic = "event_trace_breakpoint_invalid";
            return std::nullopt;
        }
        result.breakpoints.push_back(std::move(*breakpoint));
    }
    for (const auto& item : rows) {
        auto row = parseTrace(item);
        if (!row) {
            if (diagnostic) *diagnostic = "event_trace_row_invalid";
            return std::nullopt;
        }
        result.trace.push_back(std::move(*row));
    }
    return result;
}

bool PlaytestEventTraceBridge::appendControl(const EventTraceControl& control,
                                             std::string* diagnostic) const {
    if (control.control_id == 0 || control.expected_trace_revision == 0) {
        if (diagnostic) *diagnostic = "event_trace_control_invalid";
        return false;
    }
    if ((control.action == EventTraceControlAction::AddBreakpoint &&
         (control.breakpoint.id.empty() || control.breakpoint.event_id.empty() ||
          control.breakpoint.page_id.empty())) ||
        ((control.action == EventTraceControlAction::RemoveBreakpoint ||
          control.action == EventTraceControlAction::EnableBreakpoint ||
          control.action == EventTraceControlAction::DisableBreakpoint) && control.breakpoint_id.empty()) ||
        (control.action == EventTraceControlAction::WatchVariable && control.variable_id.empty())) {
        if (diagnostic) *diagnostic = "event_trace_control_payload_invalid";
        return false;
    }
    std::error_code error;
    std::filesystem::create_directories(session_directory_, error);
    if (error) {
        if (diagnostic) *diagnostic = "event_trace_control_directory_failed:" + error.message();
        return false;
    }
    std::ofstream output(session_directory_ / "event_trace_controls.jsonl",
                         std::ios::binary | std::ios::app);
    if (!output) {
        if (diagnostic) *diagnostic = "event_trace_control_open_failed";
        return false;
    }
    nlohmann::json value{{"schema", kControlSchema}, {"controlId", control.control_id},
                         {"action", actionName(control.action)},
                         {"expectedTraceRevision", control.expected_trace_revision}};
    if (control.action == EventTraceControlAction::AddBreakpoint) {
        value["breakpoint"] = breakpointJson(control.breakpoint);
    }
    if (!control.breakpoint_id.empty()) value["breakpointId"] = control.breakpoint_id;
    if (!control.variable_id.empty()) value["variableId"] = control.variable_id;
    output << value.dump() << '\n';
    return static_cast<bool>(output);
}

EventTraceControlPollResult PlaytestEventTraceBridge::pollControls(
    const ControlHandler& handler, const size_t max_controls) {
    EventTraceControlPollResult result;
    if (!handler || max_controls == 0) return result;
    std::ifstream input(session_directory_ / "event_trace_controls.jsonl", std::ios::binary);
    if (!input) return result;
    input.seekg(static_cast<std::streamoff>(consumed_control_bytes_));
    std::string line;
    while (result.processed < max_controls && std::getline(input, line)) {
        consumed_control_bytes_ += line.size() + 1;
        const auto value = nlohmann::json::parse(line, nullptr, false);
        if (!value.is_object() || value.value("schema", "") != kControlSchema) {
            ++result.processed;
            ++result.rejected;
            continue;
        }
        const auto action = parseAction(value.value("action", ""));
        EventTraceControl control;
        control.control_id = value.value("controlId", uint64_t{0});
        control.expected_trace_revision = value.value("expectedTraceRevision", uint64_t{0});
        if (!action || control.control_id == 0 || control.expected_trace_revision == 0) {
            ++result.processed;
            ++result.rejected;
            continue;
        }
        control.action = *action;
        control.breakpoint_id = value.value("breakpointId", "");
        control.variable_id = value.value("variableId", "");
        if (control.action == EventTraceControlAction::AddBreakpoint) {
            const auto breakpoint = parseBreakpoint(value.value("breakpoint", nlohmann::json::object()));
            if (!breakpoint) {
                ++result.processed;
                ++result.rejected;
                continue;
            }
            control.breakpoint = *breakpoint;
        }
        if (((control.action == EventTraceControlAction::RemoveBreakpoint ||
              control.action == EventTraceControlAction::EnableBreakpoint ||
              control.action == EventTraceControlAction::DisableBreakpoint) && control.breakpoint_id.empty()) ||
            (control.action == EventTraceControlAction::WatchVariable && control.variable_id.empty())) {
            ++result.processed;
            ++result.rejected;
            continue;
        }
        ++result.processed;
        if (handler(control)) ++result.applied;
        else ++result.rejected;
    }
    if (input.bad()) {
        result.io_error = true;
        result.error = "event_trace_control_read_failed";
    }
    return result;
}

bool applyEventTraceControl(events::EventDebugger& debugger, const EventTraceControl& control) {
    switch (control.action) {
    case EventTraceControlAction::Pause:
        debugger.pause();
        return true;
    case EventTraceControlAction::Continue:
        debugger.continueBounded();
        return true;
    case EventTraceControlAction::Step:
        (void)debugger.step();
        return true;
    case EventTraceControlAction::AdvanceFrame: return debugger.advanceFrame();
    case EventTraceControlAction::AddBreakpoint: return debugger.addConditionalBreakpoint(control.breakpoint);
    case EventTraceControlAction::RemoveBreakpoint: return debugger.removeBreakpoint(control.breakpoint_id);
    case EventTraceControlAction::EnableBreakpoint:
        return debugger.setBreakpointEnabled(control.breakpoint_id, true);
    case EventTraceControlAction::DisableBreakpoint:
        return debugger.setBreakpointEnabled(control.breakpoint_id, false);
    case EventTraceControlAction::WatchVariable:
        if (control.variable_id.empty()) return false;
        debugger.watchVariable(control.variable_id);
        return true;
    }
    return false;
}

} // namespace urpg::playtest
