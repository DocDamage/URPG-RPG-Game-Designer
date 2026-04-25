#pragma once

#include "engine/core/events/event_document.h"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace urpg::events {

struct EventDebugFrame {
    std::string event_id;
    std::string page_id;
    size_t command_index = 0;
};

struct EventDebugSnapshot {
    bool running = false;
    bool paused_on_breakpoint = false;
    std::optional<EventDebugFrame> current_frame;
    std::vector<EventDebugFrame> stack;
    std::map<std::string, int64_t> watched_variables;
};

class EventDebugger {
public:
    void start(const EventDocument& document, const std::string& event_id, const EventWorldState& state);
    void addBreakpoint(std::string event_id, std::string page_id, size_t command_index);
    bool step();
    void resume();
    void watchVariable(std::string variable_id);
    EventDebugSnapshot snapshot() const;

private:
    static std::string breakpointKey(const std::string& event_id, const std::string& page_id, size_t command_index);

    const EventDocument* document_ = nullptr;
    EventWorldState state_;
    std::vector<EventDebugFrame> stack_;
    std::set<std::string> breakpoints_;
    std::set<std::string> watched_variables_;
    bool paused_on_breakpoint_ = false;
};

} // namespace urpg::events
