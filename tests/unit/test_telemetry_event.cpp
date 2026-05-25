#include "engine/core/telemetry/telemetry_event.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TelemetryEvent serializes deterministic envelope fields", "[telemetry]") {
    urpg::telemetry::TelemetryEvent event;
    event.subsystem = "diagnostics";
    event.name = "snapshot_emitted";
    event.severity = urpg::telemetry::TelemetrySeverity::Warning;
    event.code = "diagnostics.snapshot.warning";
    event.message = "Snapshot contains warnings.";
    event.fields = {{"active_tab", "project_audit"}, {"issue_count", "2"}};

    const auto json = event.toJson();

    REQUIRE(json["schema"] == "urpg.telemetry.event.v1");
    REQUIRE(json["subsystem"] == "diagnostics");
    REQUIRE(json["name"] == "snapshot_emitted");
    REQUIRE(json["severity"] == "warning");
    REQUIRE(json["code"] == "diagnostics.snapshot.warning");
    REQUIRE(json["fields"]["active_tab"] == "project_audit");
    REQUIRE(json["fields"]["issue_count"] == "2");
}

TEST_CASE("TelemetrySink can be disabled without changing capture call success", "[telemetry]") {
    urpg::telemetry::TelemetrySink sink;
    sink.setEnabled(false);

    urpg::telemetry::TelemetryEvent event;
    event.subsystem = "runtime";
    event.name = "ignored";

    REQUIRE(sink.capture(event));
    REQUIRE(sink.events().empty());
}

TEST_CASE("Compat severity tags map into telemetry severities", "[telemetry][compat]") {
    REQUIRE(urpg::telemetry::severityFromCompatTag("WARN") == urpg::telemetry::TelemetrySeverity::Warning);
    REQUIRE(urpg::telemetry::severityFromCompatTag("SOFT_FAIL") == urpg::telemetry::TelemetrySeverity::Error);
    REQUIRE(urpg::telemetry::severityFromCompatTag("HARD_FAIL") == urpg::telemetry::TelemetrySeverity::Critical);
    REQUIRE(urpg::telemetry::severityFromCompatTag("CRASH_PREVENTED") == urpg::telemetry::TelemetrySeverity::Critical);
}
