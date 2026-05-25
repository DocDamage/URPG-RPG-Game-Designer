#include "tests/compat/compat_plugin_failure_diagnostics_helpers.h"

TEST_CASE("Compat fixtures: failure diagnostics project to telemetry envelopes",
          "[compat][fixtures][failure][telemetry]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));

    const urpg::Value missing_result = pm.executeCommand("VisuStella_CoreEngine_MZ", "missing_telemetry_command", {});
    REQUIRE(std::holds_alternative<std::monostate>(missing_result.v));

    const auto telemetry_rows = parseJsonl(pm.exportFailureTelemetryJsonl());
    REQUIRE_FALSE(telemetry_rows.empty());

    const auto& row = telemetry_rows.back();
    REQUIRE(row["schema"] == "urpg.telemetry.event.v1");
    REQUIRE(row["subsystem"] == "compat.plugin_manager");
    REQUIRE(row["name"] == "compat_failure");
    REQUIRE(row["severity"] == "critical");
    REQUIRE(row["code"] == "compat.plugin_manager.execute_command");
    REQUIRE(row["fields"]["plugin"] == "VisuStella_CoreEngine_MZ");
    REQUIRE(row["fields"]["command"] == "missing_telemetry_command");
    REQUIRE(row["fields"]["severity_tag"] == "HARD_FAIL");

    pm.clearFailureDiagnostics();
    REQUIRE(pm.exportFailureTelemetryJsonl().empty());
    pm.unloadAllPlugins();
}
