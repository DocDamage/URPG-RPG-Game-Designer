#include "editor/diagnostics/event_trace_diagnostics_panel.h"

#include <algorithm>

namespace urpg::editor {

void EventTraceDiagnosticsPanel::bind(playtest::PlaytestEventTraceBridge* bridge) {
    bridge_ = bridge;
    live_.reset();
    visible_rows_.clear();
    snapshot_ = {};
}

bool EventTraceDiagnosticsPanel::setEventFilter(std::string event_id) {
    snapshot_.event_filter = std::move(event_id);
    rebuildRows();
    return true;
}

bool EventTraceDiagnosticsPanel::setLaneFilter(std::string lane_id) {
    if (!lane_id.empty() && live_ &&
        std::find(live_->parallel_lane_ids.begin(), live_->parallel_lane_ids.end(), lane_id) ==
            live_->parallel_lane_ids.end() && lane_id != "main") {
        return false;
    }
    snapshot_.lane_filter = std::move(lane_id);
    rebuildRows();
    return true;
}

void EventTraceDiagnosticsPanel::clearFilters() {
    snapshot_.event_filter.clear();
    snapshot_.lane_filter.clear();
    rebuildRows();
}

bool EventTraceDiagnosticsPanel::selectVisibleRow(const size_t index) {
    if (index >= visible_rows_.size()) return false;
    snapshot_.selected_visible_row = index;
    snapshot_.selected_source = visible_rows_[index].source;
    return true;
}

bool EventTraceDiagnosticsPanel::openSelectedSource() {
    return snapshot_.selected_source.has_value() && source_open_handler_ &&
           source_open_handler_(*snapshot_.selected_source);
}

bool EventTraceDiagnosticsPanel::requestControl(const playtest::EventTraceControlAction action) {
    playtest::EventTraceControl control;
    control.action = action;
    return queueControl(std::move(control));
}

bool EventTraceDiagnosticsPanel::requestAddBreakpoint(events::EventBreakpoint breakpoint) {
    playtest::EventTraceControl control;
    control.action = playtest::EventTraceControlAction::AddBreakpoint;
    control.breakpoint = std::move(breakpoint);
    return queueControl(std::move(control));
}

bool EventTraceDiagnosticsPanel::requestRemoveBreakpoint(std::string breakpoint_id) {
    playtest::EventTraceControl control;
    control.action = playtest::EventTraceControlAction::RemoveBreakpoint;
    control.breakpoint_id = std::move(breakpoint_id);
    return queueControl(std::move(control));
}

bool EventTraceDiagnosticsPanel::requestBreakpointEnabled(std::string breakpoint_id, const bool enabled) {
    playtest::EventTraceControl control;
    control.action = enabled ? playtest::EventTraceControlAction::EnableBreakpoint
                             : playtest::EventTraceControlAction::DisableBreakpoint;
    control.breakpoint_id = std::move(breakpoint_id);
    return queueControl(std::move(control));
}

bool EventTraceDiagnosticsPanel::requestWatchVariable(std::string variable_id) {
    playtest::EventTraceControl control;
    control.action = playtest::EventTraceControlAction::WatchVariable;
    control.variable_id = std::move(variable_id);
    return queueControl(std::move(control));
}

bool EventTraceDiagnosticsPanel::openCurrentSource() {
    return snapshot_.current_source.has_value() && source_open_handler_ &&
           source_open_handler_(*snapshot_.current_source);
}

bool EventTraceDiagnosticsPanel::queueControl(playtest::EventTraceControl control) {
    if (bridge_ == nullptr || !live_) {
        snapshot_.last_control_code = "event_trace_not_connected";
        return false;
    }
    std::string diagnostic;
    control.control_id = next_control_id_++;
    control.expected_trace_revision = live_->revision;
    const bool appended = bridge_->appendControl(control, &diagnostic);
    snapshot_.last_control_code = appended ? "event_trace_control_queued" : std::move(diagnostic);
    return appended;
}

void EventTraceDiagnosticsPanel::refresh() {
    if (bridge_ == nullptr) {
        snapshot_.connected = false;
        return;
    }
    std::string diagnostic;
    auto next = bridge_->readAfter(live_ ? live_->revision : 0, &diagnostic);
    if (next) live_ = std::move(next);
    if (!diagnostic.empty()) snapshot_.last_control_code = std::move(diagnostic);
    snapshot_.connected = live_.has_value();
    if (!live_) return;
    snapshot_.running = live_->running;
    snapshot_.paused = live_->paused;
    snapshot_.paused_on_breakpoint = live_->paused_on_breakpoint;
    snapshot_.frame_advance_available = live_->frame_advance_available;
    snapshot_.revision = live_->revision;
    snapshot_.runtime_frame = live_->runtime_frame;
    snapshot_.total_rows = live_->trace.size();
    snapshot_.parallel_lane_ids = live_->parallel_lane_ids;
    snapshot_.call_stack = live_->stack;
    snapshot_.breakpoints = live_->breakpoints;
    snapshot_.watched_variables = live_->watched_variables;
    snapshot_.active_breakpoint_id = live_->active_breakpoint_id;
    snapshot_.current_source = live_->current_source;
    rebuildRows();
}

void EventTraceDiagnosticsPanel::render() {
    if (!visible_) return;
    refresh();
    has_rendered_frame_ = true;
}

void EventTraceDiagnosticsPanel::rebuildRows() {
    visible_rows_.clear();
    snapshot_.selected_visible_row.reset();
    snapshot_.selected_source.reset();
    if (!live_) {
        snapshot_.visible_rows = 0;
        return;
    }
    for (const auto& row : live_->trace) {
        if (!snapshot_.event_filter.empty() &&
            row.source.event_id.find(snapshot_.event_filter) == std::string::npos) {
            continue;
        }
        if (!snapshot_.lane_filter.empty() && row.lane_id != snapshot_.lane_filter) continue;
        visible_rows_.push_back(row);
    }
    snapshot_.visible_rows = visible_rows_.size();
}

} // namespace urpg::editor
