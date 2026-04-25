#include "engine/core/time/calendar_runtime.h"

#include <algorithm>

namespace urpg::time {

void CalendarRuntime::addEvent(CalendarEvent event) {
    events_.push_back(std::move(event));
}

CalendarAdvanceResult CalendarRuntime::advanceTo(int day, int hour) {
    CalendarAdvanceResult result;
    for (const auto& event : events_) {
        const bool carry_over = event.start_hour > event.end_hour && day == event.day + 1 && hour <= event.end_hour;
        const int window_day = carry_over ? event.day : day;
        const auto key = event.id + "@" + std::to_string(window_day);
        if (isInWindow(event, day, hour) && std::ranges::find(fired_keys_, key) == fired_keys_.end()) {
            result.triggeredFlags.push_back(event.flag);
            fired_keys_.push_back(key);
        }
    }
    return result;
}

std::vector<CalendarDiagnostic> CalendarRuntime::validate() const {
    std::vector<CalendarDiagnostic> diagnostics;
    for (const auto& event : events_) {
        if (event.start_hour < 0 || event.start_hour > 23 || event.end_hour < 0 || event.end_hour > 23) {
            diagnostics.push_back({"invalid_hour", "calendar event hour is outside the 0-23 range"});
        }
    }
    return diagnostics;
}

bool CalendarRuntime::isInWindow(const CalendarEvent& event, int day, int hour) const {
    if (event.start_hour <= event.end_hour) {
        return day >= event.day && hour >= event.start_hour && hour <= event.end_hour;
    }
    return (day == event.day && hour >= event.start_hour) || (day == event.day + 1 && hour <= event.end_hour);
}

} // namespace urpg::time
