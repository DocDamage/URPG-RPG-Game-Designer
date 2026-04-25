#include "engine/core/events/event_debugger.h"

namespace urpg::events {

void EventDebugger::start(const EventDocument& document, const std::string& event_id, const EventWorldState& state) {
    document_ = &document;
    state_ = state;
    stack_.clear();
    paused_on_breakpoint_ = false;

    const auto page = document.resolveActivePage(event_id, state);
    if (page.has_value()) {
        stack_.push_back(EventDebugFrame{event_id, page->id, 0});
    }
}

void EventDebugger::addBreakpoint(std::string event_id, std::string page_id, size_t command_index) {
    breakpoints_.insert(breakpointKey(event_id, page_id, command_index));
}

bool EventDebugger::step() {
    if (stack_.empty() || document_ == nullptr) {
        return false;
    }

    auto& frame = stack_.back();
    if (breakpoints_.contains(breakpointKey(frame.event_id, frame.page_id, frame.command_index)) && !paused_on_breakpoint_) {
        paused_on_breakpoint_ = true;
        return true;
    }
    paused_on_breakpoint_ = false;

    const auto page = document_->resolveActivePage(frame.event_id, state_);
    if (!page.has_value() || frame.command_index >= page->commands.size()) {
        stack_.pop_back();
        return !stack_.empty();
    }

    const auto& command = page->commands[frame.command_index];
    ++frame.command_index;
    if (command.kind == EventCommandKind::Variable && !command.target.empty()) {
        state_.variables[command.target] = command.amount;
    } else if (command.kind == EventCommandKind::Switch && !command.target.empty()) {
        state_.switches[command.target] = command.value == "true" || command.value == "on" || command.amount != 0;
    } else if (command.kind == EventCommandKind::CommonEvent && document_->commonEvents().contains(command.target)) {
        stack_.push_back(EventDebugFrame{"common:" + command.target, command.target, 0});
    }

    return !stack_.empty();
}

void EventDebugger::resume() {
    paused_on_breakpoint_ = false;
    while (step()) {
        if (paused_on_breakpoint_) {
            break;
        }
    }
}

void EventDebugger::watchVariable(std::string variable_id) {
    watched_variables_.insert(std::move(variable_id));
}

EventDebugSnapshot EventDebugger::snapshot() const {
    EventDebugSnapshot snapshot;
    snapshot.running = !stack_.empty();
    snapshot.paused_on_breakpoint = paused_on_breakpoint_;
    snapshot.stack = stack_;
    if (!stack_.empty()) {
        snapshot.current_frame = stack_.back();
    }
    for (const auto& variable : watched_variables_) {
        const auto it = state_.variables.find(variable);
        snapshot.watched_variables[variable] = it == state_.variables.end() ? 0 : it->second;
    }
    return snapshot;
}

std::string EventDebugger::breakpointKey(const std::string& event_id, const std::string& page_id, size_t command_index) {
    return event_id + "/" + page_id + "/" + std::to_string(command_index);
}

} // namespace urpg::events
