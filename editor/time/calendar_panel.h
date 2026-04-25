#pragma once

#include "engine/core/time/calendar_runtime.h"

#include <string>

namespace urpg::editor::time {

class CalendarPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::time::CalendarRuntime& runtime);
};

} // namespace urpg::editor::time
