#include "editor/compat/compat_report_panel.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using urpg::compat::PluginManager;
using urpg::compat::DataManager;

struct FixtureSpec {
    std::string pluginName;
    std::string happyPathCommand;
};

struct CapturedError {
    std::string pluginName;
    std::string commandName;
    std::string error;
};

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 &&
        sourceRoot.front() == '"' &&
        sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

std::filesystem::path fixtureDir() {
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        return sourceRoot / "tests" / "compat" / "fixtures" / "plugins";
    }
    return std::filesystem::path("tests") / "compat" / "fixtures" / "plugins";
}

std::filesystem::path fixturePath(const std::string& pluginName) {
    return fixtureDir() / (pluginName + ".json");
}

const std::vector<FixtureSpec>& fixtureSpecs() {
    static const std::vector<FixtureSpec> specs = {
        {"VisuStella_CoreEngine_MZ", "boot"},
        {"VisuStella_MainMenuCore_MZ", "openMenu"},
        {"VisuStella_OptionsCore_MZ", "openOptions"},
        {"CGMZ_MenuCommandWindow", "refresh"},
        {"CGMZ_Encyclopedia", "openCategory"},
        {"EliMZ_Book", "openBook"},
        {"MOG_CharacterMotion_MZ", "startMotion"},
        {"MOG_BattleHud_MZ", "showHud"},
        {"Galv_QuestLog_MZ", "openQuestLog"},
        {"AltMenuScreen_MZ", "applyLayout"},
    };
    return specs;
}

std::vector<nlohmann::json> parseJsonl(std::string_view jsonl) {
    std::vector<nlohmann::json> rows;
    if (jsonl.empty()) {
        return rows;
    }

    std::istringstream stream{std::string(jsonl)};
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto row = nlohmann::json::parse(line, nullptr, false);
        if (!row.is_discarded() && row.is_object()) {
            rows.push_back(std::move(row));
        }
    }
    return rows;
}

std::filesystem::path uniqueTempFixturePath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::filesystem::path temp = std::filesystem::temp_directory_path();
    return temp / (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

} // namespace

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

TEST_CASE("Compat fixtures: malformed fixture load failures are exported as diagnostics artifacts",
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

    const auto malformedFixture = uniqueTempFixturePath("urpg_malformed_fixture");
    writeTextFile(malformedFixture, "{\"name\":");

    const bool loadedMalformed = pm.loadPlugin(malformedFixture.string());
    REQUIRE_FALSE(loadedMalformed);
    REQUIRE(pm.getLastError().find("Invalid plugin fixture JSON:") != std::string::npos);

    const auto diagnosticsAfterMalformed = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnosticsAfterMalformed.empty());
    const auto& malformedRow = diagnosticsAfterMalformed.back();
    REQUIRE(malformedRow.value("operation", "") == "load_plugin_fixture_parse");
    REQUIRE(malformedRow.value("plugin", "") == malformedFixture.stem().string());

    const auto emptyEntryFixture = uniqueTempFixturePath("urpg_empty_entry_fixture");
    writeTextFile(
        emptyEntryFixture,
        R"({
  "name": "BrokenEntryFixture",
  "commands": [
    {
      "name": "brokenCommand",
      "entry": "",
      "js": "// @urpg-export brokenCommand const 1"
    }
  ]
})"
    );

    const bool loadedEmptyEntry = pm.loadPlugin(emptyEntryFixture.string());
    REQUIRE_FALSE(loadedEmptyEntry);
    REQUIRE(pm.getLastError() == "Fixture JS command entry cannot be empty: brokenCommand");

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& emptyEntryRow = diagnostics.back();
    REQUIRE(emptyEntryRow.value("operation", "") == "load_plugin_js_entry");
    REQUIRE(emptyEntryRow.value("plugin", "") == "BrokenEntryFixture");
    REQUIRE(emptyEntryRow.value("command", "") == "brokenCommand");

    REQUIRE(errors.size() >= 2);
    REQUIRE(errors[0].error.find("Invalid plugin fixture JSON:") != std::string::npos);
    REQUIRE(errors[1].pluginName == "BrokenEntryFixture");
    REQUIRE(errors[1].commandName == "brokenCommand");

    const auto evalFailureFixture = uniqueTempFixturePath("urpg_eval_failure_fixture");
    writeTextFile(
        evalFailureFixture,
        R"({
  "name": "BrokenEvalFixture",
  "commands": [
    {
      "name": "brokenEval",
      "entry": "brokenEvalRuntime",
      "js": "// @urpg-fail-eval fixture eval failure"
    }
  ]
})"
    );

    const bool loadedEvalFailure = pm.loadPlugin(evalFailureFixture.string());
    REQUIRE_FALSE(loadedEvalFailure);
    REQUIRE(pm.getLastError() == "fixture eval failure");

    const auto diagnosticsAfterEval = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnosticsAfterEval.size() >= 3);
    const auto& evalRow = diagnosticsAfterEval.back();
    REQUIRE(evalRow.value("operation", "") == "load_plugin_js_eval");
    REQUIRE(evalRow.value("plugin", "") == "BrokenEvalFixture");
    REQUIRE(evalRow.value("command", "") == "brokenEval");
    REQUIRE(evalRow.value("message", "") == "fixture eval failure");

    REQUIRE(errors.size() >= 3);
    REQUIRE(errors[2].pluginName == "BrokenEvalFixture");
    REQUIRE(errors[2].commandName == "brokenEval");
    REQUIRE(errors[2].error == "fixture eval failure");

    pm.setErrorHandler(nullptr);
    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(malformedFixture, ec);
    std::filesystem::remove(emptyEntryFixture, ec);
    std::filesystem::remove(evalFailureFixture, ec);
}

TEST_CASE("Compat fixtures: malformed fixture command payload failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto badJsPayloadFixture = uniqueTempFixturePath("urpg_bad_js_payload_fixture");
    writeTextFile(
        badJsPayloadFixture,
        R"({
  "name": "BrokenJsPayloadFixture",
  "commands": [
    {
      "name": "brokenJsPayload",
      "js": 123
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badJsPayloadFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'js' payload: brokenJsPayload");

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& badJsRow = diagnostics.back();
    REQUIRE(badJsRow.value("operation", "") == "load_plugin_js_payload");
    REQUIRE(badJsRow.value("plugin", "") == "BrokenJsPayloadFixture");
    REQUIRE(badJsRow.value("command", "") == "brokenJsPayload");

    const auto badScriptPayloadFixture = uniqueTempFixturePath("urpg_bad_script_payload_fixture");
    writeTextFile(
        badScriptPayloadFixture,
        R"({
  "name": "BrokenScriptPayloadFixture",
  "commands": [
    {
      "name": "brokenScriptPayload",
      "script": {
        "op": "set"
      }
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badScriptPayloadFixture.string()));
    REQUIRE(pm.getLastError() ==
            "Fixture script command requires array 'script' payload: brokenScriptPayload");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& badScriptRow = diagnostics.back();
    REQUIRE(badScriptRow.value("operation", "") == "load_plugin_script_payload");
    REQUIRE(badScriptRow.value("plugin", "") == "BrokenScriptPayloadFixture");
    REQUIRE(badScriptRow.value("command", "") == "brokenScriptPayload");

    const auto conflictingModeFixture = uniqueTempFixturePath("urpg_conflicting_mode_fixture");
    writeTextFile(
        conflictingModeFixture,
        R"({
  "name": "BrokenCommandModeFixture",
  "commands": [
    {
      "name": "brokenMode",
      "entry": "brokenModeRuntime",
      "js": "// @urpg-export brokenModeRuntime const 1",
      "script": [
        {"op": "return", "value": 1}
      ]
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(conflictingModeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture command cannot declare both 'js' and 'script': brokenMode");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 3);
    const auto& conflictingModeRow = diagnostics.back();
    REQUIRE(conflictingModeRow.value("operation", "") == "load_plugin_command_mode");
    REQUIRE(conflictingModeRow.value("plugin", "") == "BrokenCommandModeFixture");
    REQUIRE(conflictingModeRow.value("command", "") == "brokenMode");

    const auto badDropContextFlagFixture =
        uniqueTempFixturePath("urpg_bad_drop_context_flag_fixture");
    writeTextFile(
        badDropContextFlagFixture,
        R"({
  "name": "BrokenDropContextFlagFixture",
  "commands": [
    {
      "name": "badDropFlag",
      "entry": "badDropFlagEntry",
      "dropContextBeforeCall": "true",
      "js": "// @urpg-export badDropFlagEntry const 1"
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badDropContextFlagFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag"
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 4);
    const auto& badDropFlagRow = diagnostics.back();
    REQUIRE(badDropFlagRow.value("operation", "") == "load_plugin_drop_context_flag");
    REQUIRE(badDropFlagRow.value("plugin", "") == "BrokenDropContextFlagFixture");
    REQUIRE(badDropFlagRow.value("command", "") == "badDropFlag");

    const auto badEntryTypeFixture = uniqueTempFixturePath("urpg_bad_entry_type_fixture");
    writeTextFile(
        badEntryTypeFixture,
        R"({
  "name": "BrokenEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badEntryTypeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'entry' payload: badEntry");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 5);
    const auto& badEntryTypeRow = diagnostics.back();
    REQUIRE(badEntryTypeRow.value("operation", "") == "load_plugin_js_entry");
    REQUIRE(badEntryTypeRow.value("plugin", "") == "BrokenEntryTypeFixture");
    REQUIRE(badEntryTypeRow.value("command", "") == "badEntry");

    const auto badDescriptionTypeFixture =
        uniqueTempFixturePath("urpg_bad_description_type_fixture");
    writeTextFile(
        badDescriptionTypeFixture,
        R"({
  "name": "BrokenDescriptionTypeFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badDescriptionTypeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'description' must be string: badDescription"
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 6);
    const auto& badDescriptionTypeRow = diagnostics.back();
    REQUIRE(badDescriptionTypeRow.value("operation", "") == "load_plugin_command_description");
    REQUIRE(badDescriptionTypeRow.value("plugin", "") == "BrokenDescriptionTypeFixture");
    REQUIRE(badDescriptionTypeRow.value("command", "") == "badDescription");

    const auto unsupportedModeFixture = uniqueTempFixturePath("urpg_unsupported_mode_fixture");
    writeTextFile(
        unsupportedModeFixture,
        R"({
  "name": "BrokenUnsupportedModeFixture",
  "commands": [
    {
      "name": "badModeValue",
      "mode": "unknown_mode",
      "result": 1
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(unsupportedModeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'mode' unsupported: unknown_mode for badModeValue"
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 7);
    const auto& unsupportedModeRow = diagnostics.back();
    REQUIRE(unsupportedModeRow.value("operation", "") == "load_plugin_command_mode");
    REQUIRE(unsupportedModeRow.value("plugin", "") == "BrokenUnsupportedModeFixture");
    REQUIRE(unsupportedModeRow.value("command", "") == "badModeValue");

    const auto registerScriptFnFailureFixture =
        uniqueTempFixturePath("urpg_register_script_fn_failure_fixture");
    writeTextFile(
        registerScriptFnFailureFixture,
        R"({
  "name": "BrokenRegisterScriptFnFixture",
  "commands": [
    {
      "name": "__urpg_fail_register_function___scriptCommand",
      "script": [
        {
          "op": "return",
          "value": 1
        }
      ]
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(registerScriptFnFailureFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Failed to register QuickJS fixture function for command: __urpg_fail_register_function___scriptCommand"
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 8);
    const auto& registerScriptFnRow = diagnostics.back();
    REQUIRE(registerScriptFnRow.value("operation", "") == "load_plugin_register_script_fn");
    REQUIRE(registerScriptFnRow.value("plugin", "") == "BrokenRegisterScriptFnFixture");
    REQUIRE(
        registerScriptFnRow.value("command", "") ==
        "__urpg_fail_register_function___scriptCommand"
    );

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(badJsPayloadFixture, ec);
    std::filesystem::remove(badScriptPayloadFixture, ec);
    std::filesystem::remove(conflictingModeFixture, ec);
    std::filesystem::remove(badDropContextFlagFixture, ec);
    std::filesystem::remove(badEntryTypeFixture, ec);
    std::filesystem::remove(badDescriptionTypeFixture, ec);
    std::filesystem::remove(unsupportedModeFixture, ec);
    std::filesystem::remove(registerScriptFnFailureFixture, ec);
}

TEST_CASE("Compat fixtures: malformed fixture metadata shape failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto badDependenciesShapeFixture =
        uniqueTempFixturePath("urpg_bad_dependencies_shape_fixture");
    writeTextFile(
        badDependenciesShapeFixture,
        R"({
  "name": "BrokenDependenciesShapeFixture",
  "dependencies": "CorePlugin",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badDependenciesShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'dependencies' must be array: " + badDependenciesShapeFixture.string())
    );

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& dependenciesShapeRow = diagnostics.back();
    REQUIRE(dependenciesShapeRow.value("operation", "") == "load_plugin_dependencies");
    REQUIRE(dependenciesShapeRow.value("plugin", "") == "BrokenDependenciesShapeFixture");

    const auto badDependencyEntryFixture =
        uniqueTempFixturePath("urpg_bad_dependency_entry_fixture");
    writeTextFile(
        badDependencyEntryFixture,
        R"({
  "name": "BrokenDependencyEntryFixture",
  "dependencies": ["CorePlugin", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badDependencyEntryFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin dependency must be string at index 1: " +
         badDependencyEntryFixture.string())
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& dependencyEntryRow = diagnostics.back();
    REQUIRE(dependencyEntryRow.value("operation", "") == "load_plugin_dependency_entry");
    REQUIRE(dependencyEntryRow.value("plugin", "") == "BrokenDependencyEntryFixture");

    const auto badParametersShapeFixture =
        uniqueTempFixturePath("urpg_bad_parameters_shape_fixture");
    writeTextFile(
        badParametersShapeFixture,
        R"({
  "name": "BrokenParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badParametersShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'parameters' must be object: " + badParametersShapeFixture.string())
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 3);
    const auto& parametersShapeRow = diagnostics.back();
    REQUIRE(parametersShapeRow.value("operation", "") == "load_plugin_parameters");
    REQUIRE(parametersShapeRow.value("plugin", "") == "BrokenParametersShapeFixture");

    const auto badCommandsShapeFixture =
        uniqueTempFixturePath("urpg_bad_commands_shape_fixture");
    writeTextFile(
        badCommandsShapeFixture,
        R"({
  "name": "BrokenCommandsShapeFixture",
  "commands": {
    "name": "broken"
  }
})"
    );

    REQUIRE_FALSE(pm.loadPlugin(badCommandsShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'commands' must be array: " + badCommandsShapeFixture.string())
    );

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 4);
    const auto& commandsShapeRow = diagnostics.back();
    REQUIRE(commandsShapeRow.value("operation", "") == "load_plugin_commands");
    REQUIRE(commandsShapeRow.value("plugin", "") == "BrokenCommandsShapeFixture");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(badDependenciesShapeFixture, ec);
    std::filesystem::remove(badDependencyEntryFixture, ec);
    std::filesystem::remove(badParametersShapeFixture, ec);
    std::filesystem::remove(badCommandsShapeFixture, ec);
}

TEST_CASE("Compat fixtures: runtime QuickJS command failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto runtimeFailureFixture = uniqueTempFixturePath("urpg_runtime_failure_fixture");
    writeTextFile(
        runtimeFailureFixture,
        R"({
  "name": "BrokenRuntimeFixture",
  "commands": [
    {
      "name": "brokenRuntime",
      "entry": "brokenRuntimeEntry",
      "js": "// @urpg-fail-call brokenRuntimeEntry fixture runtime call failure"
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(runtimeFailureFixture.string()));

    const urpg::Value result = pm.executeCommand("BrokenRuntimeFixture", "brokenRuntime", {});
    REQUIRE(std::holds_alternative<std::monostate>(result.v));
    REQUIRE(pm.getLastError() == "Host function error: fixture runtime call failure");

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& runtimeFailureRow = diagnostics.back();
    REQUIRE(runtimeFailureRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(runtimeFailureRow.value("plugin", "") == "BrokenRuntimeFixture");
    REQUIRE(runtimeFailureRow.value("command", "") == "brokenRuntime");
    REQUIRE(runtimeFailureRow.value("severity", "") == "HARD_FAIL");
    REQUIRE(runtimeFailureRow.value("message", "") == "Host function error: fixture runtime call failure");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(runtimeFailureFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script runtime op failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto runtimeScriptFailureFixture =
        uniqueTempFixturePath("urpg_runtime_script_failure_fixture");
    writeTextFile(
        runtimeScriptFailureFixture,
        R"({
  "name": "BrokenScriptRuntimeFixture",
  "commands": [
    {
      "name": "scriptError",
      "script": [
        {"op": "set", "key": "command", "value": {"from": "commandName"}},
        {"op": "if",
         "condition": {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 1},
         "then": [
           {"op": "error", "message": {"from": "concat", "parts": ["script runtime failure: ", {"from": "local", "name": "command"}]}}
         ],
         "else": [
           {"op": "return", "value": "ok"}
         ]
        }
      ]
    },
    {
      "name": "unsupportedOp",
      "script": [
        {"op": "set", "key": "marker", "value": "start"},
        {"op": "unknown"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(runtimeScriptFailureFixture.string()));

    const urpg::Value scriptErrorResult =
        pm.executeCommand("BrokenScriptRuntimeFixture", "scriptError", {urpg::Value::Int(1)});
    REQUIRE(std::holds_alternative<std::monostate>(scriptErrorResult.v));
    REQUIRE(pm.getLastError() == "Host function error: script runtime failure: scriptError");

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& scriptErrorRow = diagnostics.back();
    REQUIRE(scriptErrorRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(scriptErrorRow.value("plugin", "") == "BrokenScriptRuntimeFixture");
    REQUIRE(scriptErrorRow.value("command", "") == "scriptError");
    REQUIRE(scriptErrorRow.value("message", "") ==
            "Host function error: script runtime failure: scriptError");

    const urpg::Value unsupportedOpResult =
        pm.executeCommand("BrokenScriptRuntimeFixture", "unsupportedOp", {});
    REQUIRE(std::holds_alternative<std::monostate>(unsupportedOpResult.v));
    REQUIRE(pm.getLastError().find("Host function error: Unsupported fixture script op") !=
            std::string::npos);

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& unsupportedOpRow = diagnostics.back();
    REQUIRE(unsupportedOpRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(unsupportedOpRow.value("plugin", "") == "BrokenScriptRuntimeFixture");
    REQUIRE(unsupportedOpRow.value("command", "") == "unsupportedOp");
    REQUIRE(unsupportedOpRow.value("message", "") ==
            "Host function error: Unsupported fixture script op 'unknown' at index 1");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(runtimeScriptFailureFixture, ec);
}

TEST_CASE(
    "Compat fixtures: fixture script invoke command-chain failures are exported as diagnostics artifacts",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto invokeChainFailureFixture =
        uniqueTempFixturePath("urpg_runtime_invoke_chain_failure_fixture");
    writeTextFile(
        invokeChainFailureFixture,
        R"({
  "name": "BrokenInvokeChainFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "invokeMissingRequired",
      "script": [
        {"op": "invoke", "command": "missing", "expect": "non_nil"}
      ]
    },
    {
      "name": "invokeByNameMissingRequired",
      "script": [
        {"op": "invokeByName", "name": "missingQualifiedName", "expect": "non_nil"}
      ]
    },
    {
      "name": "invokeMalformedArgs",
      "script": [
        {"op": "invoke", "command": "seed", "args": {"bad": "shape"}}
      ]
    },
    {
      "name": "invokeMalformedExpect",
      "script": [
        {"op": "invoke", "command": "seed", "expect": "unknown_expect"}
      ]
    },
    {
      "name": "invokeMalformedExpectObject",
      "script": [
        {"op": "invoke", "command": "seed", "expect": {"unsupported": true}}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(invokeChainFailureFixture.string()));

    const urpg::Value invokeMissingRequiredResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMissingRequiredResult.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op expected non-nil result for "
        "BrokenInvokeChainFixture_missing at index 0"
    );

    const urpg::Value invokeByNameMissingRequiredResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeByNameMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeByNameMissingRequiredResult.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invokeByName op expected non-nil result for "
        "missingQualifiedName at index 0"
    );

    const urpg::Value invokeMalformedArgsResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedArgs", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedArgsResult.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op requires array args at index 0"
    );

    const urpg::Value invokeMalformedExpectResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedExpect", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedExpectResult.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op unsupported expect value "
        "'unknown_expect' at index 0"
    );

      const urpg::Value invokeMalformedExpectObjectResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedExpectObject", {});
      REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedExpectObjectResult.v));
      REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op requires supported expect object at index 0"
      );

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());

    const auto invokeMissingRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "missing";
        }
    );
    REQUIRE(invokeMissingRow != diagnostics.end());
    REQUIRE(invokeMissingRow->value("severity", "") == "WARN");

    const auto invokeByNameParseRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_by_name_parse" &&
                   row.value("command", "") == "missingQualifiedName";
        }
    );
    REQUIRE(invokeByNameParseRow != diagnostics.end());
    REQUIRE(invokeByNameParseRow->value("severity", "") == "WARN");

    const auto invokeRequiredRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMissingRequired" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op expected non-nil result for "
                       "BrokenInvokeChainFixture_missing at index 0";
        }
    );
    REQUIRE(invokeRequiredRow != diagnostics.end());

    const auto invokeByNameRequiredRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeByNameMissingRequired" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invokeByName op expected non-nil result for "
                       "missingQualifiedName at index 0";
        }
    );
    REQUIRE(invokeByNameRequiredRow != diagnostics.end());

    const auto invokeMalformedArgsRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMalformedArgs" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op requires array args at index 0";
        }
    );
    REQUIRE(invokeMalformedArgsRow != diagnostics.end());

    const auto invokeMalformedExpectRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMalformedExpect" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op unsupported expect value "
                       "'unknown_expect' at index 0";
        }
    );
    REQUIRE(invokeMalformedExpectRow != diagnostics.end());

    const auto invokeMalformedExpectObjectRow = std::find_if(
      diagnostics.begin(),
      diagnostics.end(),
      [](const nlohmann::json& row) {
        return row.value("operation", "") == "execute_command_quickjs_call" &&
             row.value("plugin", "") == "BrokenInvokeChainFixture" &&
             row.value("command", "") == "invokeMalformedExpectObject" &&
             row.value("message", "") ==
               "Host function error: Fixture script invoke op requires supported expect object at index 0";
      }
    );
    REQUIRE(invokeMalformedExpectObjectRow != diagnostics.end());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(invokeChainFailureFixture, ec);
}

TEST_CASE(
    "Compat fixtures: fixture script validation failures are exported as diagnostics artifacts",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto scriptValidationFixture =
        uniqueTempFixturePath("urpg_runtime_script_validation_failure_fixture");
    writeTextFile(
        scriptValidationFixture,
        R"({
  "name": "BrokenScriptValidationFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "setMissingKey",
      "script": [
        {"op": "set", "value": 1}
      ]
    },
    {
      "name": "appendMissingKey",
      "script": [
        {"op": "append", "value": 1}
      ]
    },
    {
      "name": "invokeMissingCommand",
      "script": [
        {"op": "invoke"}
      ]
    },
    {
      "name": "invokeNonStringCommand",
      "script": [
        {"op": "invoke", "command": 7}
      ]
    },
    {
      "name": "invokeNonStringPlugin",
      "script": [
        {"op": "invoke", "plugin": 7, "command": "seed"}
      ]
    },
    {
      "name": "invokeByNameMissingName",
      "script": [
        {"op": "invokeByName"}
      ]
    },
    {
      "name": "invokeByNameNonStringName",
      "script": [
        {"op": "invokeByName", "name": 7}
      ]
    },
    {
      "name": "invokeBadStoreType",
      "script": [
        {"op": "invoke", "command": "seed", "store": 7}
      ]
    },
    {
      "name": "invokeBadExpectType",
      "script": [
        {"op": "invoke", "command": "seed", "expect": 7}
      ]
    },
    {
      "name": "errorDefaultMessage",
      "script": [
        {"op": "error"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(scriptValidationFixture.string()));

    const std::vector<std::pair<std::string, std::string>> failureCases = {
        {"setMissingKey", "Host function error: Fixture script set op requires key at index 0"},
        {"appendMissingKey",
         "Host function error: Fixture script append op requires key at index 0"},
        {"invokeMissingCommand",
         "Host function error: Fixture script invoke op requires command at index 0"},
        {"invokeNonStringCommand",
         "Host function error: Fixture script invoke op requires string command at index 0"},
        {"invokeNonStringPlugin",
         "Host function error: Fixture script invoke op requires string plugin at index 0"},
        {"invokeByNameMissingName",
         "Host function error: Fixture script invokeByName op requires name at index 0"},
        {"invokeByNameNonStringName",
         "Host function error: Fixture script invokeByName op requires string name at index 0"},
        {"invokeBadStoreType",
         "Host function error: Fixture script invoke op requires string store at index 0"},
        {"invokeBadExpectType",
            "Host function error: Fixture script invoke op requires string or object expect at index 0"},
        {"errorDefaultMessage",
         "Host function error: Fixture script error op triggered at index 0"},
    };

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const urpg::Value result = pm.executeCommand(
            "BrokenScriptValidationFixture",
            commandName,
            {}
        );
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == expectedMessage);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= failureCases.size());

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const auto rowIt = std::find_if(
            diagnostics.begin(),
            diagnostics.end(),
            [&](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command_quickjs_call" &&
                       row.value("plugin", "") == "BrokenScriptValidationFixture" &&
                       row.value("command", "") == commandName &&
                       row.value("message", "") == expectedMessage;
            }
        );
        REQUIRE(rowIt != diagnostics.end());
    }

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(scriptValidationFixture, ec);
}

TEST_CASE(
    "Compat fixtures: malformed nested-branch fixture script failures are exported as diagnostics artifacts",
    "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto nestedBranchFixture =
        uniqueTempFixturePath("urpg_runtime_nested_branch_validation_failure_fixture");
    writeTextFile(
        nestedBranchFixture,
        R"({
  "name": "BrokenNestedBranchFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "nestedThenBranchShape",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": {"op": "return", "value": "bad_shape"}}
        ]}
      ]
    },
    {
      "name": "nestedElseBranchShape",
      "script": [
        {"op": "if", "condition": false, "else": [
          {"op": "if", "condition": false, "else": {"op": "return", "value": "bad_shape"}}
        ]}
      ]
    },
    {
      "name": "nestedMissingOp",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"value": "missing-op"}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedArgs",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "args": {"bad": "shape"}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedStore",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "store": 7}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedExpectObject",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "expect": {"unsupported": true}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMissingName",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "expect": "non_nil"}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMalformedStore",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "name": "BrokenNestedBranchFixture_seed", "store": 7}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMalformedExpectObject",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "name": "BrokenNestedBranchFixture_seed", "expect": {"unsupported": true}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameBadResolverParts",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "invokeByName", "name": {"from": "concat", "parts": {"bad": "shape"}}, "expect": "non_nil"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameUnknownResolverSource",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "invokeByName", "name": {"from": "unknown_resolver", "value": "seed"}, "expect": "non_nil"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepConcatBadParts",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "append", "key": "trace", "value": {"from": "concat", "parts": {"bad": "shape"}}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepCoalesceBadValues",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "coalesced", "value": {"from": "coalesce", "values": {"bad": "shape"}}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepEqualsMissingRight",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": {"from": "equals", "left": {"from": "arg", "index": 0}}, "then": [
            {"op": "return", "value": "unreachable"}
          ], "else": [
            {"op": "return", "value": "also_unreachable"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgBadIndex",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badArg", "value": {"from": "arg", "index": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepParamBadName",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badParam", "value": {"from": "param", "name": 7}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepLocalBadName",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badLocal", "value": {"from": "local", "name": false}}
          ], "else": [
            {"op": "return", "value": "unused"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepHasArgBadIndex",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badHasArg", "value": {"from": "hasArg", "index": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepHasParamBadName",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badHasParam", "value": {"from": "hasParam", "name": 7}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgCountUnexpectedField",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badArgCount", "value": {"from": "argCount", "index": 0}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgsUnexpectedField",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badArgs", "value": {"from": "args", "name": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepParamKeysUnexpectedField",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badParamKeys", "value": {"from": "paramKeys", "index": 0}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedSetMissingKey",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "set", "value": 1}
        ]}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(nestedBranchFixture.string()));

    const std::vector<std::pair<std::string, std::string>> failureCases = {
        {"nestedThenBranchShape",
         "Host function error: Fixture script if branch must be an array at index 0"},
        {"nestedElseBranchShape",
         "Host function error: Fixture script if branch must be an array at index 0"},
        {"nestedMissingOp",
         "Host function error: Fixture script step missing op at index 0"},
        {"nestedInvokeMalformedArgs",
         "Host function error: Fixture script invoke op requires array args at index 0"},
        {"nestedInvokeMalformedStore",
         "Host function error: Fixture script invoke op requires string store at index 0"},
        {"nestedInvokeMalformedExpectObject",
         "Host function error: Fixture script invoke op requires supported expect object at index 0"},
        {"nestedInvokeByNameMissingName",
         "Host function error: Fixture script invokeByName op requires name at index 0"},
        {"nestedInvokeByNameMalformedStore",
         "Host function error: Fixture script invokeByName op requires string store at index 0"},
        {"nestedInvokeByNameMalformedExpectObject",
         "Host function error: Fixture script invokeByName op requires supported expect object at index 0"},
        {"nestedInvokeByNameBadResolverParts",
            "Host function error: Fixture script resolver concat requires array parts"},
        {"nestedInvokeByNameUnknownResolverSource",
            "Host function error: Fixture script resolver unknown source 'unknown_resolver'"},
           {"nestedDeepConcatBadParts",
            "Host function error: Fixture script resolver concat requires array parts"},
           {"nestedDeepCoalesceBadValues",
            "Host function error: Fixture script resolver coalesce requires array values"},
           {"nestedDeepEqualsMissingRight",
            "Host function error: Fixture script resolver equals requires left and right"},
          {"nestedDeepArgBadIndex",
           "Host function error: Fixture script resolver arg requires integer index"},
          {"nestedDeepParamBadName",
           "Host function error: Fixture script resolver param requires string name"},
          {"nestedDeepLocalBadName",
           "Host function error: Fixture script resolver local requires string name"},
          {"nestedDeepHasArgBadIndex",
           "Host function error: Fixture script resolver hasArg requires integer index"},
          {"nestedDeepHasParamBadName",
           "Host function error: Fixture script resolver hasParam requires string name"},
           {"nestedDeepArgCountUnexpectedField",
            "Host function error: Fixture script resolver argCount does not accept field 'index'"},
           {"nestedDeepArgsUnexpectedField",
            "Host function error: Fixture script resolver args does not accept field 'name'"},
           {"nestedDeepParamKeysUnexpectedField",
            "Host function error: Fixture script resolver paramKeys does not accept field 'index'"},
        {"nestedSetMissingKey",
         "Host function error: Fixture script set op requires key at index 0"},
    };

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const urpg::Value result = pm.executeCommand(
            "BrokenNestedBranchFixture",
            commandName,
            {}
        );
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == expectedMessage);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= failureCases.size());

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const auto rowIt = std::find_if(
            diagnostics.begin(),
            diagnostics.end(),
            [&](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command_quickjs_call" &&
                       row.value("plugin", "") == "BrokenNestedBranchFixture" &&
                       row.value("command", "") == commandName &&
                       row.value("message", "") == expectedMessage;
            }
        );
        REQUIRE(rowIt != diagnostics.end());
    }

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(nestedBranchFixture, ec);
}

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
