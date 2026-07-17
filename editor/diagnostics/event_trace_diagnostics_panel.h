#pragma once

#include "engine/core/playtest/playtest_event_trace_bridge.h"

#include <functional>
#include <optional>

namespace urpg::editor {

struct EventTraceDiagnosticsSnapshot {
    bool connected = false;
    bool running = false;
    bool paused = false;
    bool paused_on_breakpoint = false;
    bool frame_advance_available = false;
    uint64_t revision = 0;
    uint64_t runtime_frame = 0;
    size_t total_rows = 0;
    size_t visible_rows = 0;
    std::vector<std::string> parallel_lane_ids;
    std::vector<events::EventDebugFrame> call_stack;
    std::vector<events::EventBreakpoint> breakpoints;
    std::map<std::string, int64_t> watched_variables;
    std::optional<std::string> active_breakpoint_id;
    std::optional<events::EventDebugSource> current_source;
    std::string event_filter;
    std::string lane_filter;
    std::optional<size_t> selected_visible_row;
    std::optional<events::EventDebugSource> selected_source;
    std::string last_control_code;
};

class EventTraceDiagnosticsPanel {
public:
    using SourceOpenHandler = std::function<bool(const events::EventDebugSource&)>;

    void bind(playtest::PlaytestEventTraceBridge* bridge);
    void setSourceOpenHandler(SourceOpenHandler handler) { source_open_handler_ = std::move(handler); }
    bool setEventFilter(std::string event_id);
    bool setLaneFilter(std::string lane_id);
    void clearFilters();
    bool selectVisibleRow(size_t index);
    bool openSelectedSource();
    bool requestControl(playtest::EventTraceControlAction action);
    bool requestAddBreakpoint(events::EventBreakpoint breakpoint);
    bool requestRemoveBreakpoint(std::string breakpoint_id);
    bool requestBreakpointEnabled(std::string breakpoint_id, bool enabled);
    bool requestWatchVariable(std::string variable_id);
    bool openCurrentSource();
    void refresh();
    void render();

    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    const EventTraceDiagnosticsSnapshot& snapshot() const { return snapshot_; }
    const std::vector<events::EventRuntimeTraceEntry>& visibleRows() const { return visible_rows_; }

private:
    void rebuildRows();
    bool queueControl(playtest::EventTraceControl control);

    playtest::PlaytestEventTraceBridge* bridge_ = nullptr;
    std::optional<playtest::LiveEventTraceSnapshot> live_;
    std::vector<events::EventRuntimeTraceEntry> visible_rows_;
    SourceOpenHandler source_open_handler_;
    EventTraceDiagnosticsSnapshot snapshot_;
    uint64_t next_control_id_ = 1;
    bool visible_ = true;
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
