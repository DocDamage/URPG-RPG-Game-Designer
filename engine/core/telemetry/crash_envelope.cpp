#include "engine/core/telemetry/crash_envelope.h"

namespace urpg::telemetry {

nlohmann::json CrashEnvelope::toJson() const {
    nlohmann::json serializedEvents = nlohmann::json::array();
    for (const auto& event : events) {
        serializedEvents.push_back(event.toJson());
    }

    return {
        {"schema", "urpg.telemetry.crash_envelope.v1"},
        {"crash_id", crash_id},
        {"reason", reason},
        {"events", std::move(serializedEvents)},
    };
}

} // namespace urpg::telemetry
