#pragma once

#include "engine/core/events/event_document.h"
#include "engine/core/timeline/timeline_document.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::events {

struct EventMacroAction {
    int64_t tick = 0;
    std::string action;
    std::string target;
    std::string value;
};

struct EventMacroDraft {
    std::string event_id;
    std::vector<EventCommand> event_commands;
    timeline::TimelineDocument timeline;
};

class EventMacroRecorder {
public:
    void record(EventMacroAction action);
    EventMacroDraft finishDraft(const std::string& event_id) const;

private:
    std::vector<EventMacroAction> actions_;
};

} // namespace urpg::events
