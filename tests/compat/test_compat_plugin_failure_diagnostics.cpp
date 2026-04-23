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

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

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

TEST_CASE(
    "Compat fixtures: combined weekly regression covers curated dependency gating and mixed malformed/runtime chains",
    "[compat][fixtures][failure][weekly]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto fixturesRoot = fixtureDir();
    REQUIRE(std::filesystem::exists(fixturesRoot));

    const auto& specs = fixtureSpecs();
    for (const auto& spec : specs) {
        INFO("Load fixture plugin: " << spec.pluginName);
        REQUIRE(pm.loadPlugin(fixturePath(spec.pluginName).string()));
    }

    for (const auto& spec : specs) {
        INFO("Happy path command: " << spec.pluginName << "." << spec.happyPathCommand);
        const urpg::Value result = pm.executeCommand(spec.pluginName, spec.happyPathCommand, {});
        REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.v));
    }

    std::vector<std::string> executedProbePlugins;
    for (const auto& spec : specs) {
        const std::string probePlugin = "DependencyGateProbe_" + spec.pluginName;

        urpg::compat::PluginInfo info;
        info.name = probePlugin;
        info.dependencies = {spec.pluginName};
        REQUIRE(pm.registerPlugin(info));
        REQUIRE(pm.registerCommand(
            probePlugin,
            "probe",
            [&executedProbePlugins, probePlugin](const std::vector<urpg::Value>&) -> urpg::Value {
                executedProbePlugins.push_back(probePlugin);
                return urpg::Value::Int(1);
            }
        ));
    }

    for (const auto& spec : specs) {
        const std::string probePlugin = "DependencyGateProbe_" + spec.pluginName;

        INFO("Dependency gate check: " << probePlugin << " depends on " << spec.pluginName);
        REQUIRE(pm.unloadPlugin(spec.pluginName));

        const urpg::Value gated = pm.executeCommand(probePlugin, "probe", {});
        REQUIRE(std::holds_alternative<std::monostate>(gated.v));
        REQUIRE(
            std::find(executedProbePlugins.begin(), executedProbePlugins.end(), probePlugin) ==
            executedProbePlugins.end()
        );
        REQUIRE(
            pm.getLastError() ==
            ("Missing dependencies for " + probePlugin + "_probe: " + spec.pluginName)
        );
    }

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));
    REQUIRE(pm.loadPlugin(fixturePath("EliMZ_Book").string()));

    const auto weeklyLifecycleFixture =
        uniqueTempFixturePath("urpg_mixed_chain_weekly_lifecycle_fixture");
    writeTextFile(
        weeklyLifecycleFixture,
        R"({
  "name": "MixedChainWeeklyLifecycleFixture",
  "parameters": {
    "defaultRoute": "book"
  },
  "commands": [
    {
      "name": "open",
      "script": [
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 0}, {"from": "param", "name": "defaultRoute"}]}} ,
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 1, "default": "weekly_lifecycle_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "book"},
          "then": [
            {"op": "invokeByName", "name": "EliMZ_Book_openBook", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "routeResult"},
            {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "dashboard", "expect": "non_nil"}
          ],
          "else": [
            {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "dashboard", "expect": "non_nil"}
          ]
        },
        {"op": "returnObject"}
      ]
    }
  ]
})"
    );
    REQUIRE(pm.loadPlugin(weeklyLifecycleFixture.string()));

    urpg::Value weeklyBookRoute;
    weeklyBookRoute.v = std::string("book");
    urpg::Value weeklyBootArg;
    weeklyBootArg.v = std::string("weekly_lifecycle_before_unload");

    const urpg::Value weeklyLifecycleBeforeUnload = pm.executeCommand(
        "MixedChainWeeklyLifecycleFixture",
        "open",
        {weeklyBookRoute, weeklyBootArg, urpg::Value::Int(24)}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(weeklyLifecycleBeforeUnload.v));
    REQUIRE(pm.exportFailureDiagnosticsJsonl().find("MixedChainWeeklyLifecycleFixture") ==
            std::string::npos);

    REQUIRE(pm.unloadPlugin("EliMZ_Book"));

    const urpg::Value weeklyLifecycleAfterUnload = pm.executeCommand(
        "MixedChainWeeklyLifecycleFixture",
        "open",
        {weeklyBookRoute, weeklyBootArg, urpg::Value::Int(24)}
    );
    REQUIRE(std::holds_alternative<urpg::Object>(weeklyLifecycleAfterUnload.v));
    const auto& weeklyLifecycleObject = std::get<urpg::Object>(weeklyLifecycleAfterUnload.v);
    REQUIRE(std::get<std::string>(weeklyLifecycleObject.at("route").v) == "book");
    REQUIRE(std::holds_alternative<std::monostate>(weeklyLifecycleObject.at("routeResult").v));
    REQUIRE(std::holds_alternative<urpg::Object>(weeklyLifecycleObject.at("dashboard").v));
    REQUIRE(
        std::get<std::string>(
            std::get<urpg::Object>(weeklyLifecycleObject.at("dashboard").v).at("profile").v
        ) == "command_window"
    );

    const auto malformedFixture = uniqueTempFixturePath("urpg_mixed_chain_malformed_fixture");
    writeTextFile(malformedFixture, "{\"name\":");
    REQUIRE_FALSE(pm.loadPlugin(malformedFixture.string()));

    const auto missingFixture = uniqueTempFixturePath("urpg_mixed_chain_missing_fixture");
    std::filesystem::create_directories(missingFixture);
    REQUIRE(std::filesystem::exists(missingFixture));
    REQUIRE(std::filesystem::is_directory(missingFixture));
    REQUIRE(missingFixture.extension() == ".json");
    REQUIRE_FALSE(pm.loadPlugin(missingFixture.string()));
    REQUIRE(pm.getLastError().find("Failed to open plugin fixture:") != std::string::npos);

    const auto scanFailureDir =
        uniqueTempFixturePath("urpg_mixed_chain___urpg_fail_directory_scan___fixture_dir");
    std::filesystem::create_directories(scanFailureDir);
    REQUIRE(std::filesystem::exists(scanFailureDir));
    REQUIRE(std::filesystem::is_directory(scanFailureDir));
    REQUIRE(pm.loadPluginsFromDirectory(scanFailureDir.string()) == 0);
    REQUIRE(pm.getLastError().find("Failed scanning plugin directory:") != std::string::npos);

    const auto scanEntryFailureDir =
        uniqueTempFixturePath("urpg_mixed_chain_directory_entry_status_fixture_dir");
    std::filesystem::create_directories(scanEntryFailureDir);
    const auto scanEntryMarker =
        scanEntryFailureDir / "__urpg_fail_directory_entry_status___marker.json";
    writeTextFile(scanEntryMarker, "{}");
    REQUIRE(pm.loadPluginsFromDirectory(scanEntryFailureDir.string()) == 0);
    REQUIRE(pm.getLastError().find("Failed reading plugin directory entry:") !=
            std::string::npos);
    REQUIRE(pm.getLastError().find("__urpg_fail_directory_entry_status___marker.json") !=
            std::string::npos);

    const auto duplicateFixture = uniqueTempFixturePath("urpg_mixed_chain_duplicate_fixture");
    writeTextFile(
        duplicateFixture,
        R"({
  "name": "MixedChainDuplicateFixture",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE(pm.loadPlugin(duplicateFixture.string()));
    REQUIRE_FALSE(pm.loadPlugin(duplicateFixture.string()));
    REQUIRE(pm.getLastError() == "Plugin already registered: MixedChainDuplicateFixture");

    const auto emptyNameFixture = uniqueTempFixturePath("urpg_mixed_chain_empty_name_fixture");
    writeTextFile(
        emptyNameFixture,
        R"({
  "name": "",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(emptyNameFixture.string()));
    REQUIRE(pm.getLastError().find("Fixture plugin name cannot be empty:") != std::string::npos);

    REQUIRE_FALSE(pm.loadPlugin(""));
    REQUIRE(pm.getLastError() == "Plugin name cannot be empty");

    const auto duplicateCommandFixture =
        uniqueTempFixturePath("urpg_mixed_chain_register_command_fixture");
    writeTextFile(
        duplicateCommandFixture,
        R"({
  "name": "MixedChainRegisterCommandFixture",
  "commands": [
    {
      "name": "dup",
      "result": 1
    },
    {
      "name": "dup",
      "result": 2
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(duplicateCommandFixture.string()));
    REQUIRE(pm.getLastError() == "Command already registered: MixedChainRegisterCommandFixture_dup");

    const auto contextInitFailFixture =
        uniqueTempFixturePath("urpg_mixed_chain_context_init_fail_fixture");
    writeTextFile(
        contextInitFailFixture,
        R"({
  "name": "MixedChain__urpg_fail_context_init__Fixture",
  "commands": [
    {
      "name": "ctxInitFail",
      "js": "// @urpg-export ctxInitFail const 1"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(contextInitFailFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Failed to initialize QuickJS context for plugin: MixedChain__urpg_fail_context_init__Fixture"
    );

    const auto registerScriptFnFailureFixture =
        uniqueTempFixturePath("urpg_mixed_chain_register_script_fn_fixture");
    writeTextFile(
        registerScriptFnFailureFixture,
        R"({
  "name": "MixedChainRegisterScriptFnFixture",
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

    REQUIRE_FALSE(pm.parseParameters("", R"({"k":"v"})"));
    REQUIRE(pm.getLastError() == "Plugin name cannot be empty");

    REQUIRE_FALSE(pm.parseParameters("MixedChainParamsFixture", "[]"));
    REQUIRE(pm.getLastError() == "Parameter JSON must be a valid object");

    const auto dependencyShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_dependency_shape_fixture");
    writeTextFile(
        dependencyShapeFixture,
        R"({
  "name": "MixedChainDependencyShapeFixture",
  "dependencies": "VisuStella_CoreEngine_MZ",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(dependencyShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'dependencies' must be array: " + dependencyShapeFixture.string())
    );

    const auto dependencyEntryFixture =
        uniqueTempFixturePath("urpg_mixed_chain_dependency_entry_fixture");
    writeTextFile(
        dependencyEntryFixture,
        R"({
  "name": "MixedChainDependencyEntryFixture",
  "dependencies": ["VisuStella_CoreEngine_MZ", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(dependencyEntryFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin dependency must be string at index 1: " +
         dependencyEntryFixture.string())
    );

    const auto parametersShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_parameters_shape_fixture");
    writeTextFile(
        parametersShapeFixture,
        R"({
  "name": "MixedChainParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(parametersShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'parameters' must be object: " + parametersShapeFixture.string())
    );

    const auto commandsShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_commands_shape_fixture");
    writeTextFile(
        commandsShapeFixture,
        R"({
  "name": "MixedChainCommandsShapeFixture",
  "commands": {
    "name": "bad-shape"
  }
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandsShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'commands' must be array: " + commandsShapeFixture.string())
    );

    const auto payloadFixture = uniqueTempFixturePath("urpg_mixed_chain_payload_fixture");
    writeTextFile(
        payloadFixture,
        R"({
  "name": "MixedChainPayloadFixture",
  "commands": [
    {
      "name": "badPayload",
      "js": 123
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(payloadFixture.string()));

    const auto scriptPayloadFixture =
        uniqueTempFixturePath("urpg_mixed_chain_script_payload_fixture");
    writeTextFile(
        scriptPayloadFixture,
        R"({
  "name": "MixedChainScriptPayloadFixture",
  "commands": [
    {
      "name": "badScriptPayload",
      "script": {
        "op": "set"
      }
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(scriptPayloadFixture.string()));

    const auto commandShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_shape_fixture");
    writeTextFile(
        commandShapeFixture,
        R"({
  "name": "MixedChainCommandShapeFixture",
  "commands": [
    7
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandShapeFixture.string()));

    const auto commandNameFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_name_fixture");
    writeTextFile(
        commandNameFixture,
        R"({
  "name": "MixedChainCommandNameFixture",
  "commands": [
    {
      "name": "",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandNameFixture.string()));

    const auto dropContextFlagFixture =
        uniqueTempFixturePath("urpg_mixed_chain_drop_context_flag_fixture");
    writeTextFile(
        dropContextFlagFixture,
        R"({
  "name": "MixedChainDropContextFlagFixture",
  "commands": [
    {
      "name": "badDropFlag",
      "entry": "badDropFlagEntry",
      "dropContextBeforeCall": "yes",
      "js": "// @urpg-export badDropFlagEntry const 1"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(dropContextFlagFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag"
    );

    const auto entryTypeFixture = uniqueTempFixturePath("urpg_mixed_chain_entry_type_fixture");
    writeTextFile(
        entryTypeFixture,
        R"({
  "name": "MixedChainEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(entryTypeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'entry' payload: badEntry");

    const auto commandDescriptionFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_description_fixture");
    writeTextFile(
        commandDescriptionFixture,
        R"({
  "name": "MixedChainCommandDescriptionFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandDescriptionFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'description' must be string: badDescription"
    );

    const auto unsupportedModeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_unsupported_mode_fixture");
    writeTextFile(
        unsupportedModeFixture,
        R"({
  "name": "MixedChainUnsupportedModeFixture",
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

    const auto evalFixture = uniqueTempFixturePath("urpg_mixed_chain_eval_fixture");
    writeTextFile(
        evalFixture,
        R"({
  "name": "MixedChainEvalFixture",
  "commands": [
    {
      "name": "evalFail",
      "entry": "evalFailEntry",
      "js": "// @urpg-fail-eval mixed chain eval failure"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(evalFixture.string()));
    REQUIRE(pm.getLastError() == "mixed chain eval failure");

    const auto runtimeFixture = uniqueTempFixturePath("urpg_mixed_chain_runtime_fixture");
    writeTextFile(
        runtimeFixture,
        R"({
  "name": "MixedChainRuntimeFixture",
  "commands": [
    {
      "name": "runtimeSeed",
      "result": 5
    },
    {
      "name": "runtimeJsFail",
      "entry": "runtimeJsFailEntry",
      "js": "// @urpg-fail-call runtimeJsFailEntry mixed chain js failure"
    },
    {
      "name": "runtimeInvokeChainOk",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "store": "seedResult", "expect": "non_nil"},
        {"op": "invokeByName", "name": {"from": "concat", "parts": [{"from": "pluginName"}, "_runtimeSeed"]}, "store": "seedByName", "expect": "non_nil"},
        {"op": "returnObject"}
      ]
    },
    {
      "name": "runtimeInvokeMissingRequired",
      "script": [
        {"op": "invoke", "command": "runtimeMissingTarget", "expect": "non_nil"}
      ]
    },
    {
      "name": "runtimeInvokeByNameMissingRequired",
      "script": [
        {"op": "invokeByName", "name": "runtimeInvalidByName", "expect": "non_nil"}
      ]
    },
    {
      "name": "runtimeInvokeMalformedArgs",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "args": {"bad": "shape"}}
      ]
    },
    {
      "name": "runtimeInvokeMalformedExpect",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "expect": "unknown_expect"}
      ]
    },
    {
      "name": "runtimeNestedInvokeMalformedStore",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invoke", "command": "runtimeSeed", "store": 7}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeMalformedExpectObject",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invoke", "command": "runtimeSeed", "expect": {"unsupported": true}}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeByNameMalformedStore",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invokeByName", "name": "MixedChainRuntimeFixture_runtimeSeed", "store": 7}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeByNameMalformedExpectObject",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invokeByName", "name": "MixedChainRuntimeFixture_runtimeSeed", "expect": {"unsupported": true}}
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedInvokeByNameBadResolverParts",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "invokeByName", "name": {"from": "concat", "parts": {"bad": "shape"}}, "expect": "non_nil"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedInvokeByNameUnknownResolverSource",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "invokeByName", "name": {"from": "unknown_resolver", "value": "runtimeSeed"}, "expect": "non_nil"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedConcatBadParts",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "append", "key": "trace", "value": {"from": "concat", "parts": {"bad": "shape"}}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedCoalesceBadValues",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "coalesced", "value": {"from": "coalesce", "values": {"bad": "shape"}}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedEqualsMissingRight",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": {"from": "equals", "left": {"from": "arg", "index": 0}},
              "then": [
                {"op": "return", "value": "unreachable"}
              ],
              "else": [
                {"op": "return", "value": "still_unreachable"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgBadIndex",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badArg", "value": {"from": "arg", "index": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedParamBadName",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badParam", "value": {"from": "param", "name": 7}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedLocalBadName",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badLocal", "value": {"from": "local", "name": false}}
              ],
              "else": [
                {"op": "return", "value": "unused"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedHasArgBadIndex",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badHasArg", "value": {"from": "hasArg", "index": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedHasParamBadName",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badHasParam", "value": {"from": "hasParam", "name": 7}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgCountUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badArgCount", "value": {"from": "argCount", "index": 0}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgsUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badArgs", "value": {"from": "args", "name": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedParamKeysUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badParamKeys", "value": {"from": "paramKeys", "index": 0}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeScriptUnknown",
      "script": [
        {"op": "set", "key": "stage", "value": "pre"},
        {"op": "unknown"}
      ]
    },
    {
      "name": "runtimeScriptError",
      "script": [
        {"op": "error", "message": "mixed chain script error"}
      ]
    },
    {
      "name": "runtimeNestedAllAnyScriptError",
      "script": [
        {
          "op": "if",
          "condition": {
            "from": "all",
            "values": [
              {"from": "hasArg", "index": 0},
              {
                "from": "any",
                "values": [
                  {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 1},
                  {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 2}
                ]
              }
            ]
          },
          "then": [
            {"op": "error", "message": "mixed chain nested all/any script error"}
          ],
          "else": [
            {"op": "return", "value": "ok"}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedAllAnyUnknown",
      "script": [
        {
          "op": "if",
          "condition": {
            "from": "any",
            "values": [
              {
                "from": "all",
                "values": [
                  {"from": "hasArg", "index": 0},
                  {"from": "greaterThan", "left": {"from": "arg", "index": 0}, "right": 0}
                ]
              },
              {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 0}
            ]
          },
          "then": [
            {"op": "unknown"}
          ],
          "else": [
            {"op": "return", "value": "ok"}
          ]
        }
      ]
    },
    {
      "name": "runtimeContextMissingViaDrop",
      "entry": "runtimeContextMissingViaDropEntry",
      "dropContextBeforeCall": true,
      "js": "// @urpg-export runtimeContextMissingViaDropEntry const 1"
    },
    {
      "name": "runtimeMissingOp",
      "script": [
        {"value": "missing-op"}
      ]
    },
    {
      "name": "runtimeNonObjectStep",
      "script": [
        7
      ]
    },
    {
      "name": "runtimeIfBranchShape",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": {
            "op": "return",
            "value": "bad-shape"
          }
        }
      ]
    }
  ]
})"
    );
    REQUIRE(pm.loadPlugin(runtimeFixture.string()));

    const urpg::Value runtimeJsFail =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeJsFail", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeJsFail.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain js failure");

    const urpg::Value runtimeInvokeChainOk =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeChainOk", {});
    REQUIRE(std::holds_alternative<urpg::Object>(runtimeInvokeChainOk.v));
    const auto& runtimeInvokeChainObject = std::get<urpg::Object>(runtimeInvokeChainOk.v);
    REQUIRE(std::holds_alternative<int64_t>(runtimeInvokeChainObject.at("seedResult").v));
    REQUIRE(std::get<int64_t>(runtimeInvokeChainObject.at("seedResult").v) == 5);
    REQUIRE(std::holds_alternative<int64_t>(runtimeInvokeChainObject.at("seedByName").v));
    REQUIRE(std::get<int64_t>(runtimeInvokeChainObject.at("seedByName").v) == 5);

    const urpg::Value runtimeInvokeMissingRequired =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMissingRequired.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op expected non-nil result for "
        "MixedChainRuntimeFixture_runtimeMissingTarget at index 0"
    );

    const urpg::Value runtimeInvokeByNameMissingRequired =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeByNameMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeByNameMissingRequired.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invokeByName op expected non-nil result for "
        "runtimeInvalidByName at index 0"
    );

    const urpg::Value runtimeInvokeMalformedArgs =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMalformedArgs", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMalformedArgs.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op requires array args at index 0"
    );

    const urpg::Value runtimeInvokeMalformedExpect =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMalformedExpect", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMalformedExpect.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op unsupported expect value "
        "'unknown_expect' at index 0"
    );

    const urpg::Value runtimeNestedInvokeMalformedStore =
      pm.executeCommand("MixedChainRuntimeFixture", "runtimeNestedInvokeMalformedStore", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeMalformedStore.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invoke op requires string store at index 0"
    );

    const urpg::Value runtimeNestedInvokeMalformedExpectObject =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeMalformedExpectObject",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeMalformedExpectObject.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invoke op requires supported expect object at index 0"
    );

    const urpg::Value runtimeNestedInvokeByNameMalformedStore =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeByNameMalformedStore",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeByNameMalformedStore.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invokeByName op requires string store at index 0"
    );

    const urpg::Value runtimeNestedInvokeByNameMalformedExpectObject =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeByNameMalformedExpectObject",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeByNameMalformedExpectObject.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invokeByName op requires supported expect object at index 0"
    );

    const urpg::Value runtimeDeepMixedInvokeByNameBadResolverParts =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedInvokeByNameBadResolverParts",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedInvokeByNameBadResolverParts.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver concat requires array parts"
    );

    const urpg::Value runtimeDeepMixedInvokeByNameUnknownResolverSource =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedInvokeByNameUnknownResolverSource",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedInvokeByNameUnknownResolverSource.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver unknown source 'unknown_resolver'"
    );

    const urpg::Value runtimeDeepMixedConcatBadParts =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedConcatBadParts",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedConcatBadParts.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver concat requires array parts"
    );

    const urpg::Value runtimeDeepMixedCoalesceBadValues =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedCoalesceBadValues",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedCoalesceBadValues.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver coalesce requires array values"
    );

    const urpg::Value runtimeDeepMixedEqualsMissingRight =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedEqualsMissingRight",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedEqualsMissingRight.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver equals requires left and right"
    );

    const urpg::Value runtimeDeepMixedArgBadIndex =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgBadIndex",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgBadIndex.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver arg requires integer index"
    );

    const urpg::Value runtimeDeepMixedParamBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedParamBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedParamBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver param requires string name"
    );

    const urpg::Value runtimeDeepMixedLocalBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedLocalBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedLocalBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver local requires string name"
    );

    const urpg::Value runtimeDeepMixedHasArgBadIndex =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedHasArgBadIndex",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedHasArgBadIndex.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver hasArg requires integer index"
    );

    const urpg::Value runtimeDeepMixedHasParamBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedHasParamBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedHasParamBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver hasParam requires string name"
    );

    const urpg::Value runtimeDeepMixedArgCountUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgCountUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgCountUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver argCount does not accept field 'index'"
    );

    const urpg::Value runtimeDeepMixedArgsUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgsUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgsUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver args does not accept field 'name'"
    );

    const urpg::Value runtimeDeepMixedParamKeysUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedParamKeysUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedParamKeysUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver paramKeys does not accept field 'index'"
    );

    const urpg::Value runtimeScriptUnknown =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeScriptUnknown", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeScriptUnknown.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Unsupported fixture script op 'unknown' at index 1"
    );

    const urpg::Value runtimeScriptError =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeScriptError", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeScriptError.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain script error");

    const urpg::Value runtimeNestedAllAnyScriptError = pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedAllAnyScriptError",
        {urpg::Value::Int(1)}
    );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedAllAnyScriptError.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain nested all/any script error");

    const urpg::Value runtimeNestedAllAnyUnknown = pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedAllAnyUnknown",
        {urpg::Value::Int(2)}
    );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedAllAnyUnknown.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Unsupported fixture script op 'unknown' at index 0"
    );

    const urpg::Value runtimeMissingOp =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeMissingOp", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeMissingOp.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script step missing op at index 0");

    const urpg::Value runtimeNonObjectStep =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeNonObjectStep", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNonObjectStep.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script step must be an object at index 0"
    );

    const urpg::Value runtimeIfBranchShape =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeIfBranchShape", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeIfBranchShape.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script if branch must be an array at index 0"
    );

    const urpg::Value runtimeContextMissingViaDrop =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeContextMissingViaDrop", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeContextMissingViaDrop.v));
    REQUIRE(pm.getLastError() == "QuickJS context missing for plugin: MixedChainRuntimeFixture");

    const std::string invalidFullName = "mixedChainInvalidFullName";
    const urpg::Value invalidByNameResult = pm.executeCommandByName(invalidFullName, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidByNameResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);

    const std::string invalidMissingPlugin = "_mixedChainMissingPluginSegment";
    const urpg::Value invalidMissingPluginResult =
        pm.executeCommandByName(invalidMissingPlugin, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidMissingPluginResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidMissingPlugin);

    const std::string invalidMissingCommand = "mixedChainMissingCommandSegment_";
    const urpg::Value invalidMissingCommandResult =
        pm.executeCommandByName(invalidMissingCommand, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidMissingCommandResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidMissingCommand);

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());

    const auto countOperation = [&diagnostics](std::string_view operation) {
        size_t count = 0;
        for (const auto& row : diagnostics) {
            if (row.value("operation", "") == operation) {
                ++count;
            }
        }
        return count;
    };

    REQUIRE(countOperation("execute_command_dependency_missing") == specs.size());
    REQUIRE(countOperation("load_plugin_fixture_parse") >= 1);
    REQUIRE(countOperation("load_plugin_fixture_open") >= 1);
    REQUIRE(countOperation("load_plugin_fixture_name") >= 1);
    REQUIRE(countOperation("load_plugin_name") >= 1);
    REQUIRE(countOperation("load_plugin_duplicate") >= 1);
    REQUIRE(countOperation("load_plugins_directory_scan") >= 1);
    REQUIRE(countOperation("load_plugins_directory_scan_entry") >= 1);
    REQUIRE(countOperation("load_plugin_dependencies") >= 1);
    REQUIRE(countOperation("load_plugin_dependency_entry") >= 1);
    REQUIRE(countOperation("load_plugin_parameters") >= 1);
    REQUIRE(countOperation("load_plugin_commands") >= 1);
    REQUIRE(countOperation("load_plugin_js_payload") >= 1);
    REQUIRE(countOperation("load_plugin_script_payload") >= 1);
    REQUIRE(countOperation("load_plugin_command_shape") >= 1);
    REQUIRE(countOperation("load_plugin_command_name") >= 1);
    REQUIRE(countOperation("load_plugin_command_description") >= 1);
    REQUIRE(countOperation("load_plugin_drop_context_flag") >= 1);
    REQUIRE(countOperation("load_plugin_register_command") >= 1);
    REQUIRE(countOperation("load_plugin_register_script_fn") >= 1);
    REQUIRE(countOperation("load_plugin_quickjs_context") >= 1);
    REQUIRE(countOperation("load_plugin_js_entry") >= 1);
    REQUIRE(countOperation("load_plugin_js_eval") >= 1);
    REQUIRE(countOperation("parse_parameters_name") >= 1);
    REQUIRE(countOperation("parse_parameters_json") >= 1);
    REQUIRE(countOperation("execute_command") >= 2);
    REQUIRE(countOperation("execute_command_quickjs_call") >= 12);
    REQUIRE(countOperation("execute_command_quickjs_context_missing") >= 1);
    REQUIRE(countOperation("execute_command_by_name_parse") >= 4);

    for (const auto& spec : specs) {
        const std::string probePlugin = "DependencyGateProbe_" + spec.pluginName;
        const std::string expectedMessage =
            "Missing dependencies for " + probePlugin + "_probe: " + spec.pluginName;

        const auto rowIt = std::find_if(
            diagnostics.begin(),
            diagnostics.end(),
            [&](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command_dependency_missing" &&
                       row.value("plugin", "") == probePlugin &&
                       row.value("command", "") == "probe" &&
                       row.value("message", "") == expectedMessage;
            }
        );
        REQUIRE(rowIt != diagnostics.end());
    }

    const auto nestedAllAnyScriptErrorRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeNestedAllAnyScriptError" &&
                   row.value("message", "") ==
                       "Host function error: mixed chain nested all/any script error";
        }
    );
    REQUIRE(nestedAllAnyScriptErrorRow != diagnostics.end());

    const auto nestedAllAnyUnknownRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeNestedAllAnyUnknown" &&
                   row.value("message", "") ==
                       "Host function error: Unsupported fixture script op 'unknown' at index 0";
        }
    );
    REQUIRE(nestedAllAnyUnknownRow != diagnostics.end());

    const auto missingOpRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeMissingOp" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script step missing op at index 0";
        }
    );
    REQUIRE(missingOpRow != diagnostics.end());

    const auto nonObjectRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeNonObjectStep" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script step must be an object at index 0";
        }
    );
    REQUIRE(nonObjectRow != diagnostics.end());

    const auto branchShapeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeIfBranchShape" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script if branch must be an array at index 0";
        }
    );
    REQUIRE(branchShapeRow != diagnostics.end());

    const auto invokeMissingCommandRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeMissingTarget" &&
                   row.value("message", "") ==
                       "Command not found: MixedChainRuntimeFixture_runtimeMissingTarget";
        }
    );
    REQUIRE(invokeMissingCommandRow != diagnostics.end());

    const auto weeklyLifecycleMissingRow = std::find_if(
      diagnostics.begin(),
      diagnostics.end(),
      [](const nlohmann::json& row) {
        return row.value("operation", "") == "execute_command" &&
             row.value("plugin", "") == "EliMZ_Book" &&
             row.value("command", "") == "openBook" &&
             row.value("severity", "") == "WARN" &&
             row.value("message", "") ==
               "Command not found: EliMZ_Book_openBook";
      }
    );
    REQUIRE(weeklyLifecycleMissingRow != diagnostics.end());

    const auto invokeByNameParseRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_by_name_parse" &&
                   row.value("command", "") == "runtimeInvalidByName";
        }
    );
    REQUIRE(invokeByNameParseRow != diagnostics.end());

    const auto invokeMissingRequiredRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeInvokeMissingRequired" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op expected non-nil result for "
                       "MixedChainRuntimeFixture_runtimeMissingTarget at index 0";
        }
    );
    REQUIRE(invokeMissingRequiredRow != diagnostics.end());

    const auto invokeByNameMissingRequiredRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeInvokeByNameMissingRequired" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invokeByName op expected non-nil result for "
                       "runtimeInvalidByName at index 0";
        }
    );
    REQUIRE(invokeByNameMissingRequiredRow != diagnostics.end());

    const auto invokeMalformedArgsRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeInvokeMalformedArgs" &&
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
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeInvokeMalformedExpect" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op unsupported expect value "
                       "'unknown_expect' at index 0";
        }
    );
    REQUIRE(invokeMalformedExpectRow != diagnostics.end());

      const auto nestedInvokeMalformedStoreRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeNestedInvokeMalformedStore" &&
               row.value("message", "") ==
                 "Host function error: Fixture script invoke op requires string store at index 0";
        }
      );
      REQUIRE(nestedInvokeMalformedStoreRow != diagnostics.end());

      const auto nestedInvokeMalformedExpectObjectRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeNestedInvokeMalformedExpectObject" &&
               row.value("message", "") ==
                 "Host function error: Fixture script invoke op requires supported expect object at index 0";
        }
      );
      REQUIRE(nestedInvokeMalformedExpectObjectRow != diagnostics.end());

      const auto nestedInvokeByNameMalformedStoreRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeNestedInvokeByNameMalformedStore" &&
               row.value("message", "") ==
                 "Host function error: Fixture script invokeByName op requires string store at index 0";
        }
      );
      REQUIRE(nestedInvokeByNameMalformedStoreRow != diagnostics.end());

      const auto nestedInvokeByNameMalformedExpectObjectRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeNestedInvokeByNameMalformedExpectObject" &&
               row.value("message", "") ==
                 "Host function error: Fixture script invokeByName op requires supported expect object at index 0";
        }
      );
      REQUIRE(nestedInvokeByNameMalformedExpectObjectRow != diagnostics.end());

      const auto deepMixedInvokeByNameBadResolverPartsRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedInvokeByNameBadResolverParts" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver concat requires array parts";
        }
      );
      REQUIRE(deepMixedInvokeByNameBadResolverPartsRow != diagnostics.end());

      const auto deepMixedInvokeByNameUnknownResolverSourceRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedInvokeByNameUnknownResolverSource" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver unknown source 'unknown_resolver'";
        }
      );
      REQUIRE(deepMixedInvokeByNameUnknownResolverSourceRow != diagnostics.end());

      const auto deepMixedConcatBadPartsRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedConcatBadParts" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver concat requires array parts";
        }
      );
      REQUIRE(deepMixedConcatBadPartsRow != diagnostics.end());

      const auto deepMixedCoalesceBadValuesRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedCoalesceBadValues" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver coalesce requires array values";
        }
      );
      REQUIRE(deepMixedCoalesceBadValuesRow != diagnostics.end());

      const auto deepMixedEqualsMissingRightRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedEqualsMissingRight" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver equals requires left and right";
        }
      );
      REQUIRE(deepMixedEqualsMissingRightRow != diagnostics.end());

      const auto deepMixedArgBadIndexRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedArgBadIndex" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver arg requires integer index";
        }
      );
      REQUIRE(deepMixedArgBadIndexRow != diagnostics.end());

      const auto deepMixedParamBadNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedParamBadName" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver param requires string name";
        }
      );
      REQUIRE(deepMixedParamBadNameRow != diagnostics.end());

      const auto deepMixedLocalBadNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedLocalBadName" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver local requires string name";
        }
      );
      REQUIRE(deepMixedLocalBadNameRow != diagnostics.end());

      const auto deepMixedHasArgBadIndexRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedHasArgBadIndex" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver hasArg requires integer index" &&
               row.value("severity", "") == "HARD_FAIL";
        }
      );
      REQUIRE(deepMixedHasArgBadIndexRow != diagnostics.end());

      const auto deepMixedHasParamBadNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedHasParamBadName" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver hasParam requires string name" &&
               row.value("severity", "") == "HARD_FAIL";
        }
      );
      REQUIRE(deepMixedHasParamBadNameRow != diagnostics.end());

      const auto deepMixedArgCountUnexpectedFieldRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedArgCountUnexpectedField" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver argCount does not accept field 'index'" &&
               row.value("severity", "") == "HARD_FAIL";
        }
      );
      REQUIRE(deepMixedArgCountUnexpectedFieldRow != diagnostics.end());

      const auto deepMixedArgsUnexpectedFieldRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedArgsUnexpectedField" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver args does not accept field 'name'" &&
               row.value("severity", "") == "HARD_FAIL";
        }
      );
      REQUIRE(deepMixedArgsUnexpectedFieldRow != diagnostics.end());

      const auto deepMixedParamKeysUnexpectedFieldRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
          return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "MixedChainRuntimeFixture" &&
               row.value("command", "") == "runtimeDeepMixedParamKeysUnexpectedField" &&
               row.value("message", "") ==
                 "Host function error: Fixture script resolver paramKeys does not accept field 'index'" &&
               row.value("severity", "") == "HARD_FAIL";
        }
      );
      REQUIRE(deepMixedParamKeysUnexpectedFieldRow != diagnostics.end());

    const auto commandShapeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_command_shape" &&
                   row.value("plugin", "") == "MixedChainCommandShapeFixture";
        }
    );
    REQUIRE(commandShapeRow != diagnostics.end());

    const auto commandNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_command_name" &&
                   row.value("plugin", "") == "MixedChainCommandNameFixture";
        }
    );
    REQUIRE(commandNameRow != diagnostics.end());

    const auto dropContextFlagRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_drop_context_flag" &&
                   row.value("plugin", "") == "MixedChainDropContextFlagFixture" &&
                   row.value("command", "") == "badDropFlag";
        }
    );
    REQUIRE(dropContextFlagRow != diagnostics.end());

    const auto entryTypeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_js_entry" &&
                   row.value("plugin", "") == "MixedChainEntryTypeFixture" &&
                   row.value("command", "") == "badEntry";
        }
    );
    REQUIRE(entryTypeRow != diagnostics.end());

    const auto commandDescriptionRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_command_description" &&
                   row.value("plugin", "") == "MixedChainCommandDescriptionFixture" &&
                   row.value("command", "") == "badDescription";
        }
    );
    REQUIRE(commandDescriptionRow != diagnostics.end());

    const auto unsupportedModeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_command_mode" &&
                   row.value("plugin", "") == "MixedChainUnsupportedModeFixture" &&
                   row.value("command", "") == "badModeValue";
        }
    );
    REQUIRE(unsupportedModeRow != diagnostics.end());

    const auto missingFixtureRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [&](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_fixture_open" &&
                   row.value("plugin", "") == missingFixture.stem().string();
        }
    );
    REQUIRE(missingFixtureRow != diagnostics.end());

    const auto scanFailureRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugins_directory_scan";
        }
    );
    REQUIRE(scanFailureRow != diagnostics.end());

    const auto scanEntryFailureRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugins_directory_scan_entry" &&
                   row.value("message", "").find("__urpg_fail_directory_entry_status___marker.json") !=
                       std::string::npos;
        }
    );
    REQUIRE(scanEntryFailureRow != diagnostics.end());

    const auto duplicateFixtureRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_duplicate" &&
                   row.value("plugin", "") == "MixedChainDuplicateFixture";
        }
    );
    REQUIRE(duplicateFixtureRow != diagnostics.end());

    const auto emptyNameFixtureRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [&](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_fixture_name" &&
                   row.value("plugin", "") == emptyNameFixture.stem().string();
        }
    );
    REQUIRE(emptyNameFixtureRow != diagnostics.end());

    const auto parseNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "parse_parameters_name";
        }
    );
    REQUIRE(parseNameRow != diagnostics.end());

    const auto parseJsonRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "parse_parameters_json" &&
                   row.value("plugin", "") == "MixedChainParamsFixture";
        }
    );
    REQUIRE(parseJsonRow != diagnostics.end());

    const auto dependencyShapeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_dependencies" &&
                   row.value("plugin", "") == "MixedChainDependencyShapeFixture";
        }
    );
    REQUIRE(dependencyShapeRow != diagnostics.end());

    const auto dependencyEntryRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_dependency_entry" &&
                   row.value("plugin", "") == "MixedChainDependencyEntryFixture";
        }
    );
    REQUIRE(dependencyEntryRow != diagnostics.end());

    const auto parametersShapeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_parameters" &&
                   row.value("plugin", "") == "MixedChainParametersShapeFixture";
        }
    );
    REQUIRE(parametersShapeRow != diagnostics.end());

    const auto commandsShapeRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_commands" &&
                   row.value("plugin", "") == "MixedChainCommandsShapeFixture";
        }
    );
    REQUIRE(commandsShapeRow != diagnostics.end());

    const auto loadPluginNameRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_name" &&
                   row.value("plugin", "").empty();
        }
    );
    REQUIRE(loadPluginNameRow != diagnostics.end());

    const auto registerCommandRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_register_command" &&
                   row.value("plugin", "") == "MixedChainRegisterCommandFixture" &&
                   row.value("command", "") == "dup";
        }
    );
    REQUIRE(registerCommandRow != diagnostics.end());

    const auto registerScriptFnRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_register_script_fn" &&
                   row.value("plugin", "") == "MixedChainRegisterScriptFnFixture" &&
                   row.value("command", "") ==
                       "__urpg_fail_register_function___scriptCommand";
        }
    );
    REQUIRE(registerScriptFnRow != diagnostics.end());

    const auto quickjsContextRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "load_plugin_quickjs_context" &&
                   row.value("plugin", "") == "MixedChain__urpg_fail_context_init__Fixture" &&
                   row.value("command", "") == "ctxInitFail";
        }
    );
    REQUIRE(quickjsContextRow != diagnostics.end());

    const auto contextMissingRow = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_context_missing" &&
                   row.value("plugin", "") == "MixedChainRuntimeFixture" &&
                   row.value("command", "") == "runtimeContextMissingViaDrop" &&
                   row.value("message", "") ==
                       "QuickJS context missing for plugin: MixedChainRuntimeFixture";
        }
    );
    REQUIRE(contextMissingRow != diagnostics.end());

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    urpg::editor::CompatReportModel reportModel;
    reportModel.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    for (const auto& spec : specs) {
        const std::string probePlugin = "DependencyGateProbe_" + spec.pluginName;
        const auto summary = reportModel.getPluginSummary(probePlugin);
        REQUIRE(summary.unsupportedCount == 1);
        REQUIRE(summary.errorCount == 1);
        REQUIRE(summary.totalCalls == 1);

        const auto events = reportModel.getPluginEvents(probePlugin);
        REQUIRE(events.size() == 1);
        REQUIRE(events[0].methodName == "execute_command_dependency_missing");
        REQUIRE(events[0].navigationTarget == ("plugin://" + probePlugin + "#probe"));
    }

    const auto mixedRuntimeEvents = reportModel.getPluginEvents("MixedChainRuntimeFixture");
    REQUIRE(mixedRuntimeEvents.size() == 31);
    size_t quickjsCallEvents = 0;
    size_t quickjsContextMissingEvents = 0;
    size_t executeCommandWarnEvents = 0;
    for (const auto& event : mixedRuntimeEvents) {
        REQUIRE(event.navigationTarget.find("plugin://MixedChainRuntimeFixture#") == 0);
        if (event.methodName == "execute_command_quickjs_call") {
            ++quickjsCallEvents;
            continue;
        }
        if (event.methodName == "execute_command_quickjs_context_missing") {
            ++quickjsContextMissingEvents;
            continue;
        }
        if (event.methodName == "execute_command") {
            ++executeCommandWarnEvents;
            continue;
        }
        FAIL("Unexpected runtime mixed-chain event method: " + event.methodName);
    }
    REQUIRE(quickjsCallEvents == 29);
    REQUIRE(quickjsContextMissingEvents == 1);
    REQUIRE(executeCommandWarnEvents == 1);

    const auto mixedRuntimeSummary = reportModel.getPluginSummary("MixedChainRuntimeFixture");
    REQUIRE(mixedRuntimeSummary.partialCount == 1);
    REQUIRE(mixedRuntimeSummary.unsupportedCount == 2);
    REQUIRE(mixedRuntimeSummary.warningCount == 1);
    REQUIRE(mixedRuntimeSummary.errorCount == 2);
    REQUIRE(mixedRuntimeSummary.totalCalls == 31);

    const auto mixedPayloadSummary = reportModel.getPluginSummary("MixedChainPayloadFixture");
    REQUIRE(mixedPayloadSummary.unsupportedCount == 1);
    REQUIRE(mixedPayloadSummary.errorCount == 1);
    REQUIRE(mixedPayloadSummary.totalCalls == 1);

    const auto mixedScriptPayloadSummary =
        reportModel.getPluginSummary("MixedChainScriptPayloadFixture");
    REQUIRE(mixedScriptPayloadSummary.unsupportedCount == 1);
    REQUIRE(mixedScriptPayloadSummary.errorCount == 1);
    REQUIRE(mixedScriptPayloadSummary.totalCalls == 1);

    const auto mixedCommandShapeSummary =
        reportModel.getPluginSummary("MixedChainCommandShapeFixture");
    REQUIRE(mixedCommandShapeSummary.unsupportedCount == 1);
    REQUIRE(mixedCommandShapeSummary.errorCount == 1);
    REQUIRE(mixedCommandShapeSummary.totalCalls == 1);

    const auto mixedCommandNameSummary =
        reportModel.getPluginSummary("MixedChainCommandNameFixture");
    REQUIRE(mixedCommandNameSummary.unsupportedCount == 1);
    REQUIRE(mixedCommandNameSummary.errorCount == 1);
    REQUIRE(mixedCommandNameSummary.totalCalls == 1);

    const auto mixedDropContextFlagSummary =
        reportModel.getPluginSummary("MixedChainDropContextFlagFixture");
    REQUIRE(mixedDropContextFlagSummary.unsupportedCount == 1);
    REQUIRE(mixedDropContextFlagSummary.errorCount == 1);
    REQUIRE(mixedDropContextFlagSummary.totalCalls == 1);

    const auto mixedEntryTypeSummary = reportModel.getPluginSummary("MixedChainEntryTypeFixture");
    REQUIRE(mixedEntryTypeSummary.unsupportedCount == 1);
    REQUIRE(mixedEntryTypeSummary.errorCount == 1);
    REQUIRE(mixedEntryTypeSummary.totalCalls == 1);

    const auto mixedCommandDescriptionSummary =
        reportModel.getPluginSummary("MixedChainCommandDescriptionFixture");
    REQUIRE(mixedCommandDescriptionSummary.unsupportedCount == 1);
    REQUIRE(mixedCommandDescriptionSummary.errorCount == 1);
    REQUIRE(mixedCommandDescriptionSummary.totalCalls == 1);

    const auto mixedUnsupportedModeSummary =
        reportModel.getPluginSummary("MixedChainUnsupportedModeFixture");
    REQUIRE(mixedUnsupportedModeSummary.unsupportedCount == 1);
    REQUIRE(mixedUnsupportedModeSummary.errorCount == 1);
    REQUIRE(mixedUnsupportedModeSummary.totalCalls == 1);

    const auto mixedMissingFixtureSummary =
        reportModel.getPluginSummary(missingFixture.stem().string());
    REQUIRE(mixedMissingFixtureSummary.unsupportedCount == 1);
    REQUIRE(mixedMissingFixtureSummary.errorCount == 1);
    REQUIRE(mixedMissingFixtureSummary.totalCalls == 1);

    const auto mixedDuplicateFixtureSummary =
        reportModel.getPluginSummary("MixedChainDuplicateFixture");
    REQUIRE(mixedDuplicateFixtureSummary.unsupportedCount == 1);
    REQUIRE(mixedDuplicateFixtureSummary.errorCount == 1);
    REQUIRE(mixedDuplicateFixtureSummary.totalCalls == 1);

    const auto mixedEmptyNameSummary =
        reportModel.getPluginSummary(emptyNameFixture.stem().string());
    REQUIRE(mixedEmptyNameSummary.unsupportedCount == 1);
    REQUIRE(mixedEmptyNameSummary.errorCount == 1);
    REQUIRE(mixedEmptyNameSummary.totalCalls == 1);

    const auto mixedParamsSummary = reportModel.getPluginSummary("MixedChainParamsFixture");
    REQUIRE(mixedParamsSummary.unsupportedCount == 1);
    REQUIRE(mixedParamsSummary.errorCount == 1);
    REQUIRE(mixedParamsSummary.totalCalls == 1);

    const auto mixedDependencyShapeSummary =
        reportModel.getPluginSummary("MixedChainDependencyShapeFixture");
    REQUIRE(mixedDependencyShapeSummary.unsupportedCount == 1);
    REQUIRE(mixedDependencyShapeSummary.errorCount == 1);
    REQUIRE(mixedDependencyShapeSummary.totalCalls == 1);

    const auto mixedDependencyEntrySummary =
        reportModel.getPluginSummary("MixedChainDependencyEntryFixture");
    REQUIRE(mixedDependencyEntrySummary.unsupportedCount == 1);
    REQUIRE(mixedDependencyEntrySummary.errorCount == 1);
    REQUIRE(mixedDependencyEntrySummary.totalCalls == 1);

    const auto mixedParametersShapeSummary =
        reportModel.getPluginSummary("MixedChainParametersShapeFixture");
    REQUIRE(mixedParametersShapeSummary.unsupportedCount == 1);
    REQUIRE(mixedParametersShapeSummary.errorCount == 1);
    REQUIRE(mixedParametersShapeSummary.totalCalls == 1);

    const auto mixedCommandsShapeSummary =
        reportModel.getPluginSummary("MixedChainCommandsShapeFixture");
    REQUIRE(mixedCommandsShapeSummary.unsupportedCount == 1);
    REQUIRE(mixedCommandsShapeSummary.errorCount == 1);
    REQUIRE(mixedCommandsShapeSummary.totalCalls == 1);

    const auto mixedRegisterCommandSummary =
        reportModel.getPluginSummary("MixedChainRegisterCommandFixture");
    REQUIRE(mixedRegisterCommandSummary.unsupportedCount == 1);
    REQUIRE(mixedRegisterCommandSummary.errorCount == 1);
    REQUIRE(mixedRegisterCommandSummary.totalCalls == 1);

    const auto mixedRegisterScriptFnSummary =
        reportModel.getPluginSummary("MixedChainRegisterScriptFnFixture");
    REQUIRE(mixedRegisterScriptFnSummary.unsupportedCount == 1);
    REQUIRE(mixedRegisterScriptFnSummary.errorCount == 1);
    REQUIRE(mixedRegisterScriptFnSummary.totalCalls == 1);

    const auto mixedQuickJsContextSummary =
        reportModel.getPluginSummary("MixedChain__urpg_fail_context_init__Fixture");
    REQUIRE(mixedQuickJsContextSummary.unsupportedCount == 1);
    REQUIRE(mixedQuickJsContextSummary.errorCount == 1);
    REQUIRE(mixedQuickJsContextSummary.totalCalls == 1);

    const auto mixedEvalSummary = reportModel.getPluginSummary("MixedChainEvalFixture");
    REQUIRE(mixedEvalSummary.unsupportedCount == 1);
    REQUIRE(mixedEvalSummary.errorCount == 1);
    REQUIRE(mixedEvalSummary.totalCalls == 1);

    const auto weeklyLifecycleBookSummary = reportModel.getPluginSummary("EliMZ_Book");
    REQUIRE(weeklyLifecycleBookSummary.partialCount == 1);
    REQUIRE(weeklyLifecycleBookSummary.warningCount == 1);
    REQUIRE(weeklyLifecycleBookSummary.errorCount == 0);
    REQUIRE(weeklyLifecycleBookSummary.totalCalls == 1);

    const auto malformedFixtureSummary =
        reportModel.getPluginSummary(malformedFixture.stem().string());
    REQUIRE(malformedFixtureSummary.unsupportedCount == 1);
    REQUIRE(malformedFixtureSummary.errorCount == 1);
    REQUIRE(malformedFixtureSummary.totalCalls == 1);

    const std::string exportedReport = reportModel.exportAsJson();
    REQUIRE(exportedReport.find("execute_command_dependency_missing") != std::string::npos);
    REQUIRE(exportedReport.find("MixedChainRuntimeFixture") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver arg requires integer index") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver hasArg requires integer index") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver param requires string name") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver hasParam requires string name") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver local requires string name") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver argCount does not accept field 'index'") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver args does not accept field 'name'") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver paramKeys does not accept field 'index'") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver concat requires array parts") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver coalesce requires array values") != std::string::npos);
    REQUIRE(exportedReport.find("Fixture script resolver equals requires left and right") != std::string::npos);
    REQUIRE(exportedReport.find("Command not found: EliMZ_Book_openBook") != std::string::npos);

    urpg::editor::CompatReportPanel panel;
    panel.refresh();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    const auto panelRuntimeEvents = panel.getModel().getPluginEvents("MixedChainRuntimeFixture");
    REQUIRE(panelRuntimeEvents.size() == 31);
    const auto hasArgResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver arg requires integer index" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasArgResolverMessage);
    const auto hasHasArgResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver hasArg requires integer index" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasHasArgResolverMessage);
    const auto hasParamResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver param requires string name" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasParamResolverMessage);
    const auto hasHasParamResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver hasParam requires string name" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasHasParamResolverMessage);
    const auto hasLocalResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver local requires string name" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasLocalResolverMessage);
    const auto hasArgCountResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver argCount does not accept field 'index'" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasArgCountResolverMessage);
    const auto hasArgsResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver args does not accept field 'name'" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasArgsResolverMessage);
    const auto hasParamKeysResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver paramKeys does not accept field 'index'" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasParamKeysResolverMessage);
    const auto hasConcatResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver concat requires array parts" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasConcatResolverMessage);
    const auto hasCoalesceResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver coalesce requires array values" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasCoalesceResolverMessage);
    const auto hasEqualsResolverMessage = std::any_of(
      panelRuntimeEvents.begin(),
      panelRuntimeEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.message == "Host function error: Fixture script resolver equals requires left and right" &&
             event.severity == urpg::editor::CompatEvent::Severity::ERROR;
      }
    );
    REQUIRE(hasEqualsResolverMessage);

    const auto panelProbeEvents = panel.getModel().getPluginEvents(
        "DependencyGateProbe_" + specs.front().pluginName
    );
    REQUIRE(panelProbeEvents.size() == 1);
    REQUIRE(panelProbeEvents[0].methodName == "execute_command_dependency_missing");

    const auto panelWeeklyLifecycleBookEvents = panel.getModel().getPluginEvents("EliMZ_Book");
    const auto panelWeeklyLifecycleBookEventIt = std::find_if(
      panelWeeklyLifecycleBookEvents.begin(),
      panelWeeklyLifecycleBookEvents.end(),
      [](const urpg::editor::CompatEvent& event) {
        return event.methodName == "execute_command" &&
             event.message == "Command not found: EliMZ_Book_openBook" &&
             event.severity == urpg::editor::CompatEvent::Severity::WARNING;
      }
    );
    REQUIRE(panelWeeklyLifecycleBookEventIt != panelWeeklyLifecycleBookEvents.end());

    const auto panelGenericEvents = panel.getModel().getPluginEvents("");
    REQUIRE(panelGenericEvents.size() == 8);
    const auto hasByNameParse = std::any_of(
        panelGenericEvents.begin(),
        panelGenericEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "execute_command_by_name_parse";
        }
    );
    REQUIRE(hasByNameParse);
    const auto hasParseName = std::any_of(
        panelGenericEvents.begin(),
        panelGenericEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "parse_parameters_name";
        }
    );
    REQUIRE(hasParseName);
    const auto hasLoadPluginName = std::any_of(
        panelGenericEvents.begin(),
        panelGenericEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "load_plugin_name";
        }
    );
    REQUIRE(hasLoadPluginName);
    const auto hasDirectoryScan = std::any_of(
        panelGenericEvents.begin(),
        panelGenericEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "load_plugins_directory_scan";
        }
    );
    REQUIRE(hasDirectoryScan);
    const auto hasDirectoryScanEntry = std::any_of(
        panelGenericEvents.begin(),
        panelGenericEvents.end(),
        [](const urpg::editor::CompatEvent& event) {
            return event.methodName == "load_plugins_directory_scan_entry";
        }
    );
    REQUIRE(hasDirectoryScanEntry);

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove_all(missingFixture, ec);
    std::filesystem::remove_all(scanFailureDir, ec);
    std::filesystem::remove_all(scanEntryFailureDir, ec);
    std::filesystem::remove(malformedFixture, ec);
    std::filesystem::remove(duplicateFixture, ec);
    std::filesystem::remove(emptyNameFixture, ec);
    std::filesystem::remove(duplicateCommandFixture, ec);
    std::filesystem::remove(contextInitFailFixture, ec);
    std::filesystem::remove(registerScriptFnFailureFixture, ec);
    std::filesystem::remove(dependencyShapeFixture, ec);
    std::filesystem::remove(dependencyEntryFixture, ec);
    std::filesystem::remove(parametersShapeFixture, ec);
    std::filesystem::remove(commandsShapeFixture, ec);
    std::filesystem::remove(payloadFixture, ec);
    std::filesystem::remove(scriptPayloadFixture, ec);
    std::filesystem::remove(commandShapeFixture, ec);
    std::filesystem::remove(commandNameFixture, ec);
    std::filesystem::remove(dropContextFlagFixture, ec);
    std::filesystem::remove(weeklyLifecycleFixture, ec);
    std::filesystem::remove(entryTypeFixture, ec);
    std::filesystem::remove(commandDescriptionFixture, ec);
    std::filesystem::remove(unsupportedModeFixture, ec);
    std::filesystem::remove(evalFixture, ec);
    std::filesystem::remove(runtimeFixture, ec);
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
