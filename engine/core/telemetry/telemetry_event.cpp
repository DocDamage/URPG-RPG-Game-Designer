#include "engine/core/telemetry/telemetry_event.h"

namespace urpg::telemetry {

std::string toString(TelemetrySeverity severity) {
    switch (severity) {
    case TelemetrySeverity::Info:
        return "info";
    case TelemetrySeverity::Warning:
        return "warning";
    case TelemetrySeverity::Error:
        return "error";
    case TelemetrySeverity::Critical:
        return "critical";
    }
    return "info";
}

TelemetrySeverity severityFromCompatTag(const std::string& tag) {
    if (tag == "WARN") {
        return TelemetrySeverity::Warning;
    }
    if (tag == "SOFT_FAIL") {
        return TelemetrySeverity::Error;
    }
    if (tag == "HARD_FAIL" || tag == "CRASH_PREVENTED") {
        return TelemetrySeverity::Critical;
    }
    return TelemetrySeverity::Info;
}

nlohmann::json TelemetryEvent::toJson() const {
    return {
        {"schema", "urpg.telemetry.event.v1"},
        {"subsystem", subsystem},
        {"name", name},
        {"severity", toString(severity)},
        {"code", code},
        {"message", message},
        {"fields", fields},
    };
}

void TelemetrySink::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool TelemetrySink::enabled() const {
    return enabled_;
}

bool TelemetrySink::capture(const TelemetryEvent& event) {
    if (enabled_) {
        events_.push_back(event);
    }
    return true;
}

const std::vector<TelemetryEvent>& TelemetrySink::events() const {
    return events_;
}

} // namespace urpg::telemetry
