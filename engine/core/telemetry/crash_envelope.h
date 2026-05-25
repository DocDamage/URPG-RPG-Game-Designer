#pragma once

#include "engine/core/telemetry/telemetry_event.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::telemetry {

struct CrashEnvelope {
    std::string crash_id;
    std::string reason;
    std::vector<TelemetryEvent> events;

    [[nodiscard]] nlohmann::json toJson() const;
};

} // namespace urpg::telemetry
