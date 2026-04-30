#include "tests/compat/compat_plugin_failure_diagnostics_helpers.h"

TEST_CASE(
    "Compat fixtures: curated plugin profiles emit deterministic missing-command diagnostics",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto fixturesRoot = fixtureDir();
    REQUIRE(std::filesystem::exists(fixturesRoot));

    for (const auto& spec : fixtureSpecs()) {
        INFO("Load fixture plugin: " << spec.pluginName);
        REQUIRE(pm.loadPlugin(fixturePath(spec.pluginName).string()));
    }

    std::vector<CapturedError> errors;
    pm.setErrorHandler([&errors](const std::string& pluginName,
                                 const std::string& commandName,
                                 const std::string& error) {
        errors.push_back(CapturedError{pluginName, commandName, error});
    });

    for (const auto& spec : fixtureSpecs()) {
        INFO("Plugin profile: " << spec.pluginName);

        const urpg::Value okResult =
            pm.executeCommand(spec.pluginName, spec.happyPathCommand, {});
        REQUIRE_FALSE(std::holds_alternative<std::monostate>(okResult.v));

        const std::string missingCommand = spec.happyPathCommand + "_missing";
        const urpg::Value missingResult = pm.executeCommand(
            spec.pluginName,
            missingCommand,
            {}
        );
        REQUIRE(std::holds_alternative<std::monostate>(missingResult.v));

        const std::string expected =
            "Command not found: " + spec.pluginName + "_" + missingCommand;
        REQUIRE(pm.getLastError() == expected);
    }

    REQUIRE(errors.size() == fixtureSpecs().size());
    for (size_t i = 0; i < fixtureSpecs().size(); ++i) {
        const auto& spec = fixtureSpecs()[i];
        const auto& error = errors[i];
        const std::string expectedCommand = spec.happyPathCommand + "_missing";
        const std::string expectedMessage =
            "Command not found: " + spec.pluginName + "_" + expectedCommand;

        REQUIRE(error.pluginName == spec.pluginName);
        REQUIRE(error.commandName == expectedCommand);
        REQUIRE(error.error == expectedMessage);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= fixtureSpecs().size());
    const size_t firstFailureRow = diagnostics.size() - fixtureSpecs().size();
    for (size_t i = 0; i < fixtureSpecs().size(); ++i) {
        const auto& row = diagnostics[firstFailureRow + i];
        const auto& spec = fixtureSpecs()[i];
        REQUIRE(row.value("subsystem", "") == "plugin_manager");
        REQUIRE(row.value("event", "") == "compat_failure");
        REQUIRE(row.value("plugin", "") == spec.pluginName);
        REQUIRE(row.value("command", "") == spec.happyPathCommand + "_missing");
        REQUIRE(row.value("operation", "") == "execute_command");
    }

    pm.setErrorHandler(nullptr);
    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}

TEST_CASE("Compat fixtures: invalid full-name command parse is surfaced to diagnostics",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    std::vector<CapturedError> errors;
    pm.setErrorHandler([&errors](const std::string& pluginName,
                                 const std::string& commandName,
                                 const std::string& error) {
        errors.push_back(CapturedError{pluginName, commandName, error});
    });

    const std::vector<std::string> invalidFullNames = {
        "notAQualifiedCommandName",
        "_missingPluginSegment",
        "missingCommandSegment_",
    };

    for (const auto& invalidFullName : invalidFullNames) {
        const urpg::Value result = pm.executeCommandByName(invalidFullName, {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);
    }

    REQUIRE(errors.size() == invalidFullNames.size());
    for (size_t i = 0; i < invalidFullNames.size(); ++i) {
        REQUIRE(errors[i].pluginName.empty());
        REQUIRE(errors[i].commandName == invalidFullNames[i]);
        REQUIRE(errors[i].error == "Invalid command name format: " + invalidFullNames[i]);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() == invalidFullNames.size());
    for (size_t i = 0; i < invalidFullNames.size(); ++i) {
        REQUIRE(diagnostics[i].value("operation", "") == "execute_command_by_name_parse");
        REQUIRE(diagnostics[i].value("command", "") == invalidFullNames[i]);
        REQUIRE(diagnostics[i].value("severity", "") == "WARN");
    }

    pm.setErrorHandler(nullptr);
    pm.clearFailureDiagnostics();
}

TEST_CASE(
    "Compat fixtures: curated dependent profiles report missing core dependency after unload",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));

    REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    REQUIRE(pm.unloadPlugin("VisuStella_CoreEngine_MZ"));

    REQUIRE_FALSE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE_FALSE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    const auto missingMainMenu = pm.getMissingDependencies("VisuStella_MainMenuCore_MZ");
    REQUIRE(missingMainMenu.size() == 1);
    REQUIRE(missingMainMenu.front() == "VisuStella_CoreEngine_MZ");

    const auto missingOptions = pm.getMissingDependencies("VisuStella_OptionsCore_MZ");
    REQUIRE(missingOptions.size() == 1);
    REQUIRE(missingOptions.front() == "VisuStella_CoreEngine_MZ");

    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();
}

TEST_CASE(
    "Compat fixtures: dependent command execution is gated with diagnostics when core dependency is missing",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));

    REQUIRE(pm.unloadPlugin("VisuStella_CoreEngine_MZ"));

    const urpg::Value result = pm.executeCommand(
        "VisuStella_MainMenuCore_MZ",
        "openMenu",
        {}
    );
    REQUIRE(std::holds_alternative<std::monostate>(result.v));
    REQUIRE(
        pm.getLastError() ==
        "Missing dependencies for VisuStella_MainMenuCore_MZ_openMenu: VisuStella_CoreEngine_MZ"
    );

    const urpg::Value byNameResult = pm.executeCommandByName(
        "VisuStella_MainMenuCore_MZ_openMenu",
        {}
    );
    REQUIRE(std::holds_alternative<std::monostate>(byNameResult.v));
    REQUIRE(
        pm.getLastError() ==
        "Missing dependencies for VisuStella_MainMenuCore_MZ_openMenu: VisuStella_CoreEngine_MZ"
    );

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);

    const auto dependencyFailureCount = std::count_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const auto& row) {
            return row.value("operation", "") == "execute_command_dependency_missing" &&
                   row.value("plugin", "") == "VisuStella_MainMenuCore_MZ" &&
                   row.value("command", "") == "openMenu" &&
                   row.value("severity", "") == "SOFT_FAIL" &&
                   row.value("message", "") ==
                       "Missing dependencies for VisuStella_MainMenuCore_MZ_openMenu: VisuStella_CoreEngine_MZ";
        }
    );
    REQUIRE(dependencyFailureCount >= 2);

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}

TEST_CASE(
    "Compat fixtures: curated presentation lifecycle failures project through diagnostics report and panel",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto lifecycleFixture = uniqueTempFixturePath("urpg_curated_presentation_failure_fixture");
    writeTextFile(
        lifecycleFixture,
        R"({
  "name": "CuratedPresentationFailureFixture",
  "parameters": {
    "defaultRoute": "motion"
  },
  "commands": [
    {
      "name": "run",
      "script": [
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 0}, {"from": "param", "name": "defaultRoute"}]}} ,
        {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 1, "default": "idle"}},
        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "layout"},
          "then": [
            {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "applyLayout", "store": "routeResult"}
          ],
          "else": [
            {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "hud"},
              "then": [
                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult"}
              ],
              "else": [
                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "routeResult"}
              ]
            }
          ]
        },
        {"op": "returnObject"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(lifecycleFixture.string()));

    urpg::Value motionRoute;
    motionRoute.v = std::string("motion");
    urpg::Value motionName;
    motionName.v = std::string("dash");

    const urpg::Value beforeUnload = pm.executeCommand(
        "CuratedPresentationFailureFixture",
        "run",
      {motionRoute, motionName}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(beforeUnload.v));
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    REQUIRE(pm.unloadPlugin("MOG_CharacterMotion_MZ"));
    REQUIRE_FALSE(pm.isPluginLoaded("MOG_CharacterMotion_MZ"));

    const urpg::Value afterUnload = pm.executeCommand(
        "CuratedPresentationFailureFixture",
        "run",
      {motionRoute, motionName}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(afterUnload.v));
    const auto& afterUnloadObject = std::get<urpg::Object>(afterUnload.v);
    REQUIRE(std::get<std::string>(afterUnloadObject.at("route").v) == "motion");
    REQUIRE(std::holds_alternative<std::monostate>(afterUnloadObject.at("routeResult").v));

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto motionMissingRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command" &&
                   row.value("plugin", "") == "MOG_CharacterMotion_MZ" &&
                   row.value("command", "") == "startMotion" &&
                   row.value("severity", "") == "WARN" &&
                   row.value("message", "") ==
                       "Command not found: MOG_CharacterMotion_MZ_startMotion";
        }
    );
    REQUIRE(motionMissingRow != diagnostics.end());

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto motionEvents = reportModel.getPluginEvents("MOG_CharacterMotion_MZ");
    const auto motionEventIt = std::find_if(
        motionEvents.begin(),
        motionEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "execute_command" &&
                   event.message == "Command not found: MOG_CharacterMotion_MZ_startMotion" &&
                   event.severity == urpg::editor::CompatEvent::Severity::WARNING &&
                   event.navigationTarget == "plugin://MOG_CharacterMotion_MZ#startMotion";
        }
    );
    REQUIRE(motionEventIt != motionEvents.end());

    const auto motionSummary = reportModel.getPluginSummary("MOG_CharacterMotion_MZ");
    REQUIRE(motionSummary.warningCount == 1);
    REQUIRE(motionSummary.errorCount == 0);
    REQUIRE(motionSummary.totalCalls == 1);

    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("MOG_CharacterMotion_MZ") != std::string::npos);
    REQUIRE(exportedReport.find("Command not found: MOG_CharacterMotion_MZ_startMotion") != std::string::npos);

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    const auto panelMotionEvents = panel.getModel().getPluginEvents("MOG_CharacterMotion_MZ");
    const auto panelMotionEventIt = std::find_if(
        panelMotionEvents.begin(),
        panelMotionEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "execute_command" &&
                   event.message == "Command not found: MOG_CharacterMotion_MZ_startMotion" &&
                   event.severity == urpg::editor::CompatEvent::Severity::WARNING;
        }
    );
    REQUIRE(panelMotionEventIt != panelMotionEvents.end());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(lifecycleFixture, ec);
}

TEST_CASE(
    "Compat fixtures: curated save-data lifecycle failures project through diagnostics report and panel",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    DataManager& data = DataManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();
    pm.clearExecutionDiagnostics();

    data.setupNewGame();
    data.deleteSaveFile(0);
    data.deleteSaveFile(1);

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));

    const auto lifecycleFixture = uniqueTempFixturePath("urpg_curated_save_data_failure_fixture");
    writeTextFile(
        lifecycleFixture,
        R"({
  "name": "CuratedSaveDataFailureFixture",
  "parameters": {
    "defaultRoute": "slot",
    "defaultToken": "party"
  },
  "commands": [
    {
      "name": "open",
      "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "save_failure_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "menu"},
        {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "dashboard"},
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "autosave"},
          "then": [
            {"op": "set", "key": "routeToken", "value": {"from": "arg", "index": 2, "default": "autosave"}}
          ],
          "else": [
            {"op": "set", "key": "routeToken", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultToken"}}}
          ]
        },
        {"op": "returnObject"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(lifecycleFixture.string()));

    data.setGold(450);
    data.gainItem(2, 7);
    data.setVariable(4, 88);
    data.setPlayerPosition(9, 10, 11);
    data.setPlayerDirection(6);
    REQUIRE(data.saveGame(1));

    urpg::Value slotToken;
    slotToken.v = std::string("party");
    REQUIRE(data.setSaveHeaderExtension(1, "ui.tab", slotToken));

    data.setAutosaveEnabled(true);
    data.setVariable(8, 144);
    REQUIRE(data.saveAutosave());

    urpg::Value slotRoute;
    slotRoute.v = std::string("slot");
    urpg::Value autosaveRoute;
    autosaveRoute.v = std::string("autosave");
    urpg::Value beforeSlotBoot;
    beforeSlotBoot.v = std::string("before_failure_slot");
    urpg::Value beforeAutosaveBoot;
    beforeAutosaveBoot.v = std::string("before_failure_autosave");
    urpg::Value autosaveToken;
    autosaveToken.v = std::string("autosave");

    const urpg::Value beforeSlot = pm.executeCommand(
        "CuratedSaveDataFailureFixture",
        "open",
        {beforeSlotBoot, slotRoute, slotToken}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(beforeSlot.v));

    const urpg::Value beforeAutosave = pm.executeCommandByName(
        "CuratedSaveDataFailureFixture_open",
        {beforeAutosaveBoot, autosaveRoute, autosaveToken}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(beforeAutosave.v));
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    REQUIRE(pm.unloadPlugin("CGMZ_MenuCommandWindow"));
    REQUIRE_FALSE(pm.isPluginLoaded("CGMZ_MenuCommandWindow"));

    urpg::Value afterSlotBoot;
    afterSlotBoot.v = std::string("after_failure_slot");
    const urpg::Value afterSlot = pm.executeCommand(
        "CuratedSaveDataFailureFixture",
        "open",
        {afterSlotBoot, slotRoute, slotToken}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(afterSlot.v));
    const auto& afterSlotObject = std::get<urpg::Object>(afterSlot.v);
    REQUIRE(std::get<std::string>(afterSlotObject.at("route").v) == "slot");
    REQUIRE(std::get<std::string>(afterSlotObject.at("routeToken").v) == "party");
    REQUIRE(std::holds_alternative<std::monostate>(afterSlotObject.at("dashboard").v));

    urpg::Value afterAutosaveBoot;
    afterAutosaveBoot.v = std::string("after_failure_autosave");
    const urpg::Value afterAutosave = pm.executeCommandByName(
        "CuratedSaveDataFailureFixture_open",
        {afterAutosaveBoot, autosaveRoute, autosaveToken}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(afterAutosave.v));
    const auto& afterAutosaveObject = std::get<urpg::Object>(afterAutosave.v);
    REQUIRE(std::get<std::string>(afterAutosaveObject.at("route").v) == "autosave");
    REQUIRE(std::get<std::string>(afterAutosaveObject.at("routeToken").v) == "autosave");
    REQUIRE(std::holds_alternative<std::monostate>(afterAutosaveObject.at("dashboard").v));

    REQUIRE(data.loadGame(1));
    REQUIRE(data.getGold() == 450);
    REQUIRE(data.getItemCount(2) == 7);
    REQUIRE(data.getVariable(4) == 88);
    auto savedTab = data.getSaveHeaderExtension(1, "ui.tab");
    REQUIRE(savedTab.has_value());
    REQUIRE(std::holds_alternative<std::string>(savedTab->v));
    REQUIRE(std::get<std::string>(savedTab->v) == "party");
    REQUIRE(data.loadAutosave());
    REQUIRE(data.getVariable(8) == 144);

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto dashboardRows = std::count_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command" &&
                   row.value("plugin", "") == "CGMZ_MenuCommandWindow" &&
                   row.value("command", "") == "refresh" &&
                   row.value("severity", "") == "WARN" &&
                   row.value("message", "") ==
                       "Command not found: CGMZ_MenuCommandWindow_refresh";
        }
    );
    REQUIRE(dashboardRows == 2);

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto dashboardEvents = reportModel.getPluginEvents("CGMZ_MenuCommandWindow");
    REQUIRE(dashboardEvents.size() == 2);
    for (const auto& event : dashboardEvents) {
        REQUIRE(event.methodName == "execute_command");
        REQUIRE(event.message == "Command not found: CGMZ_MenuCommandWindow_refresh");
        REQUIRE(event.severity == urpg::editor::CompatEvent::Severity::WARNING);
        REQUIRE(event.navigationTarget == "plugin://CGMZ_MenuCommandWindow#refresh");
    }

    const auto dashboardSummary = reportModel.getPluginSummary("CGMZ_MenuCommandWindow");
    REQUIRE(dashboardSummary.warningCount == 1);
    REQUIRE(dashboardSummary.errorCount == 0);
    REQUIRE(dashboardSummary.totalCalls == 2);

    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("CGMZ_MenuCommandWindow") != std::string::npos);
    REQUIRE(exportedReport.find("Command not found: CGMZ_MenuCommandWindow_refresh") != std::string::npos);

    pm.clearExecutionDiagnostics();
    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
    REQUIRE(pm.exportExecutionDiagnosticsJsonl().empty());

    const auto panelDashboardEvents = panel.getModel().getPluginEvents("CGMZ_MenuCommandWindow");
    REQUIRE(panelDashboardEvents.size() == 2);
    const auto panelDashboardEventCount = std::count_if(
        panelDashboardEvents.begin(),
        panelDashboardEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "execute_command" &&
                   event.message == "Command not found: CGMZ_MenuCommandWindow_refresh" &&
                   event.severity == urpg::editor::CompatEvent::Severity::WARNING;
        }
    );
    REQUIRE(panelDashboardEventCount == 2);

    data.deleteSaveFile(0);
    data.deleteSaveFile(1);
    pm.clearFailureDiagnostics();
    pm.clearExecutionDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(lifecycleFixture, ec);
}
