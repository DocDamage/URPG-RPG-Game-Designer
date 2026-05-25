#include "engine/core/telemetry/crash_envelope.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CrashEnvelope serializes deterministic crash context", "[telemetry][crash_envelope]") {
    urpg::telemetry::TelemetryEvent event;
    event.subsystem = "compat";
    event.name = "plugin_failure";
    event.severity = urpg::telemetry::TelemetrySeverity::Critical;
    event.code = "compat.plugin_failure";
    event.message = "Plugin command failed.";

    urpg::telemetry::CrashEnvelope envelope;
    envelope.crash_id = "crash-001";
    envelope.reason = "Unhandled compat plugin failure";
    envelope.events = {event};

    const auto json = envelope.toJson();

    REQUIRE(json["schema"] == "urpg.telemetry.crash_envelope.v1");
    REQUIRE(json["crash_id"] == "crash-001");
    REQUIRE(json["reason"] == "Unhandled compat plugin failure");
    REQUIRE(json["events"].size() == 1);
    REQUIRE(json["events"][0]["severity"] == "critical");
}
