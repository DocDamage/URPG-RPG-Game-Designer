#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using urpg::compat::PluginManager;

struct FixtureSpec {
    std::string pluginName;
    std::string happyPathCommand;
};

struct CapturedError {
    std::string pluginName;
    std::string commandName;
    std::string error;
};

std::filesystem::path fixtureDir() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR) / "tests" / "compat" / "fixtures" / "plugins";
#else
    return std::filesystem::path("tests") / "compat" / "fixtures" / "plugins";
#endif
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

    const std::string invalidFullName = "notAQualifiedCommandName";
    const urpg::Value result = pm.executeCommandByName(invalidFullName, {});
    REQUIRE(std::holds_alternative<std::monostate>(result.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);

    REQUIRE(errors.size() == 1);
    REQUIRE(errors.front().pluginName.empty());
    REQUIRE(errors.front().commandName == invalidFullName);
    REQUIRE(errors.front().error == pm.getLastError());

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics.front().value("operation", "") == "execute_command_by_name_parse");
    REQUIRE(diagnostics.front().value("command", "") == invalidFullName);

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

    pm.clearFailureDiagnostics();
    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
}
