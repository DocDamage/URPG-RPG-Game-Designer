#include "editor/time/calendar_panel.h"

namespace urpg::editor::time {

std::string CalendarPanel::snapshot(const urpg::time::CalendarRuntime& runtime) {
    return runtime.validate().empty() ? "calendar:ready" : "calendar:diagnostics";
}

} // namespace urpg::editor::time
