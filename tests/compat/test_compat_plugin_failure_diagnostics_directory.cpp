#include "tests/compat/compat_plugin_failure_diagnostics_helpers.h"

TEST_CASE("Compat fixtures: missing directory scan failure is captured in diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto missingDir = uniqueTempFixturePath("urpg_missing_fixture_dir");
    const auto loadedCount = pm.loadPluginsFromDirectory(missingDir.string());
    REQUIRE(loadedCount == 0);
    REQUIRE(pm.getLastError().find("Plugin directory not found:") != std::string::npos);

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics.front().value("operation", "") == "load_plugins_directory");
    REQUIRE(diagnostics.front().value("plugin", "").empty());
    REQUIRE(diagnostics.front().value("command", "").empty());
    REQUIRE(diagnostics.front().value("severity", "") == "HARD_FAIL");

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto reportEvents = reportModel.getPluginEvents("");
    REQUIRE(reportEvents.size() == 1);
    REQUIRE(reportEvents.front().methodName == "load_plugins_directory");
    REQUIRE(reportEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);
    REQUIRE(reportEvents.front().message.find("Plugin directory not found:") == 0);
    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("load_plugins_directory") != std::string::npos);

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
    const auto panelEvents = panel.getModel().getPluginEvents("");
    REQUIRE(panelEvents.size() == 1);
    REQUIRE(panelEvents.front().methodName == "load_plugins_directory");
    REQUIRE(panelEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);

    pm.clearFailureDiagnostics();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
}

TEST_CASE("Compat fixtures: deterministic directory iterator scan failure is captured in diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto scanFailureDir =
        uniqueTempFixturePath("urpg___urpg_fail_directory_scan___fixture_dir");
    std::filesystem::create_directories(scanFailureDir);
    REQUIRE(std::filesystem::exists(scanFailureDir));
    REQUIRE(std::filesystem::is_directory(scanFailureDir));

    const auto loadedCount = pm.loadPluginsFromDirectory(scanFailureDir.string());
    REQUIRE(loadedCount == 0);
    REQUIRE(pm.getLastError() == "Failed scanning plugin directory: " + scanFailureDir.string());

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics.front().value("operation", "") == "load_plugins_directory_scan");
    REQUIRE(diagnostics.front().value("plugin", "").empty());
    REQUIRE(diagnostics.front().value("command", "").empty());
    REQUIRE(diagnostics.front().value("severity", "") == "HARD_FAIL");

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto reportEvents = reportModel.getPluginEvents("");
    REQUIRE(reportEvents.size() == 1);
    REQUIRE(reportEvents.front().methodName == "load_plugins_directory_scan");
    REQUIRE(reportEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);
    REQUIRE(reportEvents.front().message.find("Failed scanning plugin directory: ") == 0);
    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("load_plugins_directory_scan") != std::string::npos);

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
    const auto panelEvents = panel.getModel().getPluginEvents("");
    REQUIRE(panelEvents.size() == 1);
    REQUIRE(panelEvents.front().methodName == "load_plugins_directory_scan");
    REQUIRE(panelEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove_all(scanFailureDir, ec);
}

TEST_CASE("Compat fixtures: deterministic directory entry status failure is captured in diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto scanEntryFailureDir =
        uniqueTempFixturePath("urpg_directory_entry_status_fixture_dir");
    std::filesystem::create_directories(scanEntryFailureDir);
    const auto markerEntry =
        scanEntryFailureDir / "__urpg_fail_directory_entry_status___marker.json";
    writeTextFile(markerEntry, "{}");

    const auto loadedCount = pm.loadPluginsFromDirectory(scanEntryFailureDir.string());
    REQUIRE(loadedCount == 0);
    REQUIRE(pm.getLastError().find("Failed reading plugin directory entry:") == 0);
    REQUIRE(pm.getLastError().find("__urpg_fail_directory_entry_status___marker.json") !=
            std::string::npos);

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics.front().value("operation", "") == "load_plugins_directory_scan_entry");
    REQUIRE(diagnostics.front().value("message", "").find(
                "__urpg_fail_directory_entry_status___marker.json") != std::string::npos);
    REQUIRE(diagnostics.front().value("plugin", "").empty());
    REQUIRE(diagnostics.front().value("command", "").empty());
    REQUIRE(diagnostics.front().value("severity", "") == "HARD_FAIL");

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto reportEvents = reportModel.getPluginEvents("");
    REQUIRE(reportEvents.size() == 1);
    REQUIRE(reportEvents.front().methodName == "load_plugins_directory_scan_entry");
    REQUIRE(reportEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);
    REQUIRE(reportEvents.front().message.find(
                "__urpg_fail_directory_entry_status___marker.json") != std::string::npos);
    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("load_plugins_directory_scan_entry") != std::string::npos);

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
    const auto panelEvents = panel.getModel().getPluginEvents("");
    REQUIRE(panelEvents.size() == 1);
    REQUIRE(panelEvents.front().methodName == "load_plugins_directory_scan_entry");
    REQUIRE(panelEvents.front().severity == urpg::editor::CompatEvent::Severity::ERROR);

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove_all(scanEntryFailureDir, ec);
}

TEST_CASE(
    "Compat fixtures: dependency drift fixture loads and exposes declared dependency list",
    "[compat][fixtures][health][s26]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto driftFixturePath = fixturePath("URPG_DependencyDrift_Fixture");
    REQUIRE(std::filesystem::exists(driftFixturePath));

    // Load the known dependency first so loading succeeds
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    // The drift fixture declares a dependency on URPG_NonExistent_DependencyTarget
    // which is intentionally absent from the corpus. The fixture should load
    // (because missing dependencies are reported at execution time, not load time),
    // but checkDependencies must return false.
    REQUIRE(pm.loadPlugin(driftFixturePath.string()));
    REQUIRE(pm.isPluginLoaded("URPG_DependencyDrift_Fixture"));

    // The missing dependency should be detected
    const auto missing = pm.getMissingDependencies("URPG_DependencyDrift_Fixture");
    REQUIRE_FALSE(missing.empty());
    const bool hasDriftDep = std::find(missing.begin(), missing.end(),
        "URPG_NonExistent_DependencyTarget") != missing.end();
    REQUIRE(hasDriftDep);

    // checkDependencies returns false because one dependency is absent
    REQUIRE_FALSE(pm.checkDependencies("URPG_DependencyDrift_Fixture"));

    // Executing a command should produce a dependency-missing diagnostic
    pm.clearFailureDiagnostics();
    const urpg::Value result = pm.executeCommand(
        "URPG_DependencyDrift_Fixture",
        "runWithDependencies",
        {}
    );
    REQUIRE(std::holds_alternative<std::monostate>(result.v));

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const bool hasDriftDiagnostic = std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_dependency_missing" &&
                   row.value("plugin", "") == "URPG_DependencyDrift_Fixture" &&
                   row.value("severity", "") == "SOFT_FAIL";
        }
    );
    REQUIRE(hasDriftDiagnostic);

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}

TEST_CASE(
    "Compat fixtures: profile mismatch fixture loads and produces expected parameter snapshot",
    "[compat][fixtures][health][s26]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto mismatchFixturePath = fixturePath("URPG_ProfileMismatch_Fixture");
    REQUIRE(std::filesystem::exists(mismatchFixturePath));

    REQUIRE(pm.loadPlugin(mismatchFixturePath.string()));
    REQUIRE(pm.isPluginLoaded("URPG_ProfileMismatch_Fixture"));

    // checkProfileMetadata command returns the paramKeys list
    const urpg::Value keysResult = pm.executeCommand(
        "URPG_ProfileMismatch_Fixture",
        "checkProfileMetadata",
        {}
    );
    // The result should be an array of parameter key strings
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(keysResult.v));

    // runWithMismatchedProfile command should succeed and surface the profile value
    const urpg::Value runResult = pm.executeCommand(
        "URPG_ProfileMismatch_Fixture",
        "runWithMismatchedProfile",
        {}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(runResult.v));
    const auto& resultObj = std::get<urpg::Object>(runResult.v);

    // The profile parameter should carry the unconventional uppercase value as declared
    const auto profileIt = resultObj.find("profile");
    REQUIRE(profileIt != resultObj.end());
    REQUIRE(std::holds_alternative<std::string>(profileIt->second.v));
    CHECK(std::get<std::string>(profileIt->second.v) ==
          "INVALID_PROFILE_NAME_UPPERCASE_VIOLATES_CONVENTION");

    // No failure diagnostics should be generated by a valid (if unconventional) execution
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}

TEST_CASE(
    "Compat fixtures: JSONL diagnostic rows carry all required health-check fields",
    "[compat][fixtures][health][s26]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));

    // Trigger a known missing-command failure to produce a JSONL row
    const urpg::Value result = pm.executeCommand(
        "VisuStella_CoreEngine_MZ",
        "boot_missing_for_health_check",
        {}
    );
    REQUIRE(std::holds_alternative<std::monostate>(result.v));

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());

    // Every JSONL row must carry the full required field set for health-check audit
    const std::vector<std::string> requiredFields = {
        "subsystem", "event", "plugin", "command", "operation", "severity", "message"
    };
    for (const auto& row : diagnostics) {
        for (const auto& field : requiredFields) {
            INFO("Field '" << field << "' must be present in JSONL row");
            REQUIRE(row.contains(field));
            REQUIRE_FALSE(row.value(field, "").empty());
        }
    }

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}
