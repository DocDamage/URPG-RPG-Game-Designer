#pragma once

#include <string>
#include <vector>

namespace urpg::time {

struct CalendarDiagnostic {
    std::string code;
    std::string message;
};

struct CalendarEvent {
    std::string id;
    int day{1};
    int start_hour{0};
    int end_hour{0};
    std::string flag;
};

struct CalendarAdvanceResult {
    std::vector<std::string> triggeredFlags;
};

class CalendarRuntime {
public:
    void addEvent(CalendarEvent event);
    [[nodiscard]] CalendarAdvanceResult advanceTo(int day, int hour);
    [[nodiscard]] std::vector<CalendarDiagnostic> validate() const;

private:
    [[nodiscard]] bool isInWindow(const CalendarEvent& event, int day, int hour) const;

    std::vector<CalendarEvent> events_;
    std::vector<std::string> fired_keys_;
};

} // namespace urpg::time
