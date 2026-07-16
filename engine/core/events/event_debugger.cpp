#include "engine/core/events/event_debugger.h"

#include <algorithm>

namespace urpg::events {

void EventDebugger::start(const EventDocument& document, const std::string& event_id, const EventWorldState& state) {
    document_ = &document;
    state_ = state;
    native_state_ = {};
    native_state_.switches = state.switches;
    native_state_.variables = state.variables;
    stack_.clear();
    trace_.clear();
    parallel_lane_ids_.clear();
    next_trace_sequence_ = 1;
    paused_on_breakpoint_ = false;
    paused_manually_ = false;
    active_breakpoint_id_.reset();
    runtime_frame_ = 0;

    const auto page = document.resolveActivePage(event_id, state);
    if (page.has_value()) {
        stack_.push_back(EventDebugFrame{event_id, page->id, 0});
    }
}

void EventDebugger::addBreakpoint(std::string event_id, std::string page_id, size_t command_index) {
    const auto id = breakpointKey(event_id, page_id, command_index);
    addConditionalBreakpoint(EventBreakpoint{id, std::move(event_id), std::move(page_id), command_index, {}});
}

bool EventDebugger::addConditionalBreakpoint(EventBreakpoint breakpoint) {
    if (breakpoint.id.empty() || breakpoint.event_id.empty() || breakpoint.page_id.empty()) return false;
    return breakpoints_.emplace(breakpoint.id, std::move(breakpoint)).second;
}

bool EventDebugger::setBreakpointEnabled(const std::string& breakpoint_id, const bool enabled) {
    const auto found = breakpoints_.find(breakpoint_id);
    if (found == breakpoints_.end()) return false;
    found->second.enabled = enabled;
    if (!enabled && active_breakpoint_id_ == breakpoint_id) {
        paused_on_breakpoint_ = false;
        active_breakpoint_id_.reset();
    }
    return true;
}

bool EventDebugger::removeBreakpoint(const std::string& breakpoint_id) {
    if (active_breakpoint_id_ == breakpoint_id) {
        paused_on_breakpoint_ = false;
        active_breakpoint_id_.reset();
    }
    return breakpoints_.erase(breakpoint_id) != 0;
}

void EventDebugger::pause() {
    if (!stack_.empty()) paused_manually_ = true;
}

bool EventDebugger::step() {
    if (stack_.empty() || document_ == nullptr) {
        return false;
    }

    auto& frame = stack_.back();
    if (paused_on_breakpoint_) {
        paused_on_breakpoint_ = false;
        active_breakpoint_id_.reset();
    } else if (auto* breakpoint = matchingBreakpoint(frame)) {
        ++breakpoint->hit_count;
        paused_on_breakpoint_ = true;
        active_breakpoint_id_ = breakpoint->id;
        return true;
    }

    const auto* commands = commandsForFrame(frame);
    if (commands == nullptr || frame.command_index >= commands->size()) {
        stack_.pop_back();
        return !stack_.empty();
    }

    const auto command_index = frame.command_index;
    const auto command = (*commands)[command_index];
    ++frame.command_index;

    const auto old_switch = command.kind == EventCommandKind::Switch ? native_state_.switches[command.target] : false;
    const auto old_variable = command.kind == EventCommandKind::Variable ? native_state_.variables[command.target] : int64_t{0};
    const auto result = executeNativeEventCommand(command, native_state_);
    state_.switches = native_state_.switches;
    state_.variables = native_state_.variables;

    EventRuntimeTraceEntry trace;
    trace.sequence = next_trace_sequence_++;
    trace.source = sourceFor(frame, command);
    trace.source.command_index = command_index;
    trace.command_kind = command.kind;
    trace.stack_depth = stack_.size();
    trace.result_code = result.code;
    if (command.kind == EventCommandKind::Condition || command.kind == EventCommandKind::ElseBranch) {
        trace.branch_taken = result.branch_taken;
    }
    if (result.waits) trace.wait_frames = native_state_.wait_frames;
    if (command.kind == EventCommandKind::Switch && result.success) {
        trace.mutations.push_back({"switch", command.target, old_switch ? "true" : "false",
                                   native_state_.switches[command.target] ? "true" : "false"});
    } else if (command.kind == EventCommandKind::Variable && result.success) {
        trace.mutations.push_back({"variable", command.target, std::to_string(old_variable),
                                   std::to_string(native_state_.variables[command.target])});
    }
    if (command.kind == EventCommandKind::Parallel && result.success) {
        trace.lane_id = "parallel:" + command.id;
        parallel_lane_ids_.push_back(trace.lane_id);
    }
    trace_.push_back(std::move(trace));
    constexpr size_t kTraceLimit = 4096;
    if (trace_.size() > kTraceLimit) trace_.erase(trace_.begin());

    if (command.kind == EventCommandKind::CommonEvent && document_->commonEvents().contains(command.target)) {
        stack_.push_back(EventDebugFrame{"common:" + command.target, command.target, 0});
    }

    return !stack_.empty();
}

bool EventDebugger::advanceFrame() {
    if ((!paused_manually_ && !paused_on_breakpoint_) || native_state_.wait_frames <= 0) return false;
    --native_state_.wait_frames;
    ++runtime_frame_;
    return true;
}

void EventDebugger::continueExecution() {
    paused_manually_ = false;
    while (step()) {
        if (paused_on_breakpoint_ || paused_manually_) break;
    }
}

void EventDebugger::resume() {
    continueExecution();
}

void EventDebugger::watchVariable(std::string variable_id) {
    watched_variables_.insert(std::move(variable_id));
}

EventDebugSnapshot EventDebugger::snapshot() const {
    EventDebugSnapshot snapshot;
    snapshot.running = !stack_.empty();
    snapshot.paused_on_breakpoint = paused_on_breakpoint_;
    snapshot.paused_manually = paused_manually_;
    snapshot.frame_advance_available = (paused_manually_ || paused_on_breakpoint_) && native_state_.wait_frames > 0;
    snapshot.runtime_frame = runtime_frame_;
    snapshot.active_breakpoint_id = active_breakpoint_id_;
    snapshot.stack = stack_;
    if (!stack_.empty()) {
        snapshot.current_frame = stack_.back();
        const auto* commands = commandsForFrame(stack_.back());
        if (commands != nullptr && stack_.back().command_index < commands->size()) {
            snapshot.current_command = (*commands)[stack_.back().command_index];
            snapshot.current_source = sourceFor(stack_.back(), *snapshot.current_command);
        }
    }
    for (const auto& variable : watched_variables_) {
        const auto it = state_.variables.find(variable);
        snapshot.watched_variables[variable] = it == state_.variables.end() ? 0 : it->second;
    }
    snapshot.branch_depth = native_state_.branch_depth;
    snapshot.wait_frames = native_state_.wait_frames;
    snapshot.parallel_lane_ids = parallel_lane_ids_;
    snapshot.trace = trace_;
    for (const auto& [id, breakpoint] : breakpoints_) snapshot.breakpoints.push_back(breakpoint);
    return snapshot;
}

std::optional<EventDebugSource> EventDebugger::sourceForTrace(const uint64_t sequence) const {
    const auto entry = std::find_if(trace_.begin(), trace_.end(), [&](const EventRuntimeTraceEntry& candidate) {
        return candidate.sequence == sequence;
    });
    return entry == trace_.end() ? std::nullopt : std::optional<EventDebugSource>{entry->source};
}

std::optional<EventDebugSource> EventDebugger::sourceForBreakpoint(const std::string& breakpoint_id) const {
    const auto found = breakpoints_.find(breakpoint_id);
    if (found == breakpoints_.end()) return std::nullopt;
    const EventDebugFrame frame{found->second.event_id, found->second.page_id, found->second.command_index};
    const auto* commands = commandsForFrame(frame);
    if (commands == nullptr || frame.command_index >= commands->size()) return std::nullopt;
    return sourceFor(frame, (*commands)[frame.command_index]);
}

bool EventDebugger::conditionMatches(const EventBreakpointCondition& condition) const {
    switch (condition.kind) {
    case EventBreakpointConditionKind::Always:
        return true;
    case EventBreakpointConditionKind::VariableEquals: {
        const auto found = native_state_.variables.find(condition.target);
        return (found == native_state_.variables.end() ? 0 : found->second) == condition.number_value;
    }
    case EventBreakpointConditionKind::VariableAtLeast: {
        const auto found = native_state_.variables.find(condition.target);
        return (found == native_state_.variables.end() ? 0 : found->second) >= condition.number_value;
    }
    case EventBreakpointConditionKind::SwitchEquals: {
        const auto found = native_state_.switches.find(condition.target);
        return (found == native_state_.switches.end() ? false : found->second) == condition.bool_value;
    }
    case EventBreakpointConditionKind::SelfSwitchEquals: {
        const auto found = native_state_.self_switches.find(condition.target);
        return (found == native_state_.self_switches.end() ? false : found->second) == condition.bool_value;
    }
    }
    return false;
}

EventBreakpoint* EventDebugger::matchingBreakpoint(const EventDebugFrame& frame) {
    for (auto& [id, breakpoint] : breakpoints_) {
        if (breakpoint.enabled && breakpoint.event_id == frame.event_id && breakpoint.page_id == frame.page_id &&
            breakpoint.command_index == frame.command_index && conditionMatches(breakpoint.condition)) {
            return &breakpoint;
        }
    }
    return nullptr;
}

const std::vector<EventCommand>* EventDebugger::commandsForFrame(const EventDebugFrame& frame) const {
    if (document_ == nullptr) return nullptr;
    if (frame.event_id.starts_with("common:")) {
        const auto common = document_->commonEvents().find(frame.page_id);
        return common == document_->commonEvents().end() ? nullptr : &common->second.commands;
    }
    const auto event = std::find_if(document_->events().begin(), document_->events().end(),
                                    [&](const EventDefinition& candidate) { return candidate.id == frame.event_id; });
    if (event == document_->events().end()) return nullptr;
    const auto page = std::find_if(event->pages.begin(), event->pages.end(),
                                   [&](const EventPage& candidate) { return candidate.id == frame.page_id; });
    return page == event->pages.end() ? nullptr : &page->commands;
}

EventDebugSource EventDebugger::sourceFor(const EventDebugFrame& frame, const EventCommand& command) {
    return {frame.event_id.starts_with("common:") ? "common_event_editor" : "event_editor",
            frame.event_id, frame.page_id, command.id, frame.command_index};
}

std::string EventDebugger::breakpointKey(const std::string& event_id, const std::string& page_id, size_t command_index) {
    return event_id + "/" + page_id + "/" + std::to_string(command_index);
}

} // namespace urpg::events
