#include "editor/compat/compat_report_panel.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using urpg::compat::PluginManager;

struct FixtureSpec {
    std::string pluginName;
    std::string happyPathCommand;
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

using DiagnosticRows = std::vector<nlohmann::json>;

size_t countOperation(const DiagnosticRows& diagnostics, std::string_view operation) {
    size_t count = 0;
    for (const auto& row : diagnostics) {
        if (row.value("operation", "") == operation) {
            ++count;
        }
    }
    return count;
}

bool hasDiagnosticRow(const DiagnosticRows& diagnostics, const nlohmann::json& expected) {
    for (const auto& row : diagnostics) {
        bool matches = true;
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            if (!it.value().is_string() || row.value(it.key(), "") != it.value().get<std::string>()) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return true;
        }
    }
    return false;
}

void requireDiagnosticRow(const DiagnosticRows& diagnostics, const nlohmann::json& expected) {
    INFO("Expected diagnostic row: " << expected.dump());
    REQUIRE(hasDiagnosticRow(diagnostics, expected));
}

void verifyOperationCounts(const DiagnosticRows& diagnostics, size_t dependencyProbeCount) {
    REQUIRE(countOperation(diagnostics, "execute_command_dependency_missing") == dependencyProbeCount);
    REQUIRE(countOperation(diagnostics, "load_plugin_fixture_parse") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_fixture_open") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_fixture_name") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_name") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_duplicate") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugins_directory_scan") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugins_directory_scan_entry") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_dependencies") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_dependency_entry") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_parameters") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_commands") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_js_payload") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_script_payload") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_command_shape") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_command_name") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_command_description") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_drop_context_flag") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_register_command") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_register_script_fn") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_quickjs_context") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_js_entry") >= 1);
    REQUIRE(countOperation(diagnostics, "load_plugin_js_eval") >= 1);
    REQUIRE(countOperation(diagnostics, "parse_parameters_name") >= 1);
    REQUIRE(countOperation(diagnostics, "parse_parameters_json") >= 1);
    REQUIRE(countOperation(diagnostics, "execute_command") >= 2);
    REQUIRE(countOperation(diagnostics, "execute_command_quickjs_call") >= 12);
    REQUIRE(countOperation(diagnostics, "execute_command_quickjs_context_missing") >= 1);
    REQUIRE(countOperation(diagnostics, "execute_command_by_name_parse") >= 4);
}

void verifyDependencyGateDiagnosticRows(const DiagnosticRows& diagnostics, const std::vector<FixtureSpec>& specs) {
    for (const auto& spec : specs) {
        const std::string probePlugin = "DependencyGateProbe_" + spec.pluginName;
        requireDiagnosticRow(
            diagnostics,
            {
                {"operation", "execute_command_dependency_missing"},
                {"plugin", probePlugin},
                {"command", "probe"},
                {"message", "Missing dependencies for " + probePlugin + "_probe: " + spec.pluginName}
            }
        );
    }
}

void verifyRuntimeDiagnosticRows(const DiagnosticRows& diagnostics) {
    const auto requireRuntimeRow =
        [&diagnostics](std::string command, std::string message, std::string severity = "") {
            nlohmann::json expected{
                {"operation", "execute_command_quickjs_call"},
                {"plugin", "MixedChainRuntimeFixture"},
                {"command", std::move(command)},
                {"message", std::move(message)}
            };
            if (!severity.empty()) {
                expected["severity"] = std::move(severity);
            }
            requireDiagnosticRow(diagnostics, expected);
        };

    requireRuntimeRow(
        "runtimeNestedAllAnyScriptError",
        "Host function error: mixed chain nested all/any script error"
    );
    requireRuntimeRow(
        "runtimeNestedAllAnyUnknown",
        "Host function error: Unsupported fixture script op 'unknown' at index 0"
    );
    requireRuntimeRow(
        "runtimeMissingOp",
        "Host function error: Fixture script step missing op at index 0"
    );
    requireRuntimeRow(
        "runtimeNonObjectStep",
        "Host function error: Fixture script step must be an object at index 0"
    );
    requireRuntimeRow(
        "runtimeIfBranchShape",
        "Host function error: Fixture script if branch must be an array at index 0"
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "execute_command"},
            {"plugin", "MixedChainRuntimeFixture"},
            {"command", "runtimeMissingTarget"},
            {"message", "Command not found: MixedChainRuntimeFixture_runtimeMissingTarget"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "execute_command"},
            {"plugin", "EliMZ_Book"},
            {"command", "openBook"},
            {"severity", "WARN"},
            {"message", "Command not found: EliMZ_Book_openBook"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "execute_command_by_name_parse"},
            {"command", "runtimeInvalidByName"}
        }
    );
    requireRuntimeRow(
        "runtimeInvokeMissingRequired",
        "Host function error: Fixture script invoke op expected non-nil result for "
        "MixedChainRuntimeFixture_runtimeMissingTarget at index 0"
    );
    requireRuntimeRow(
        "runtimeInvokeByNameMissingRequired",
        "Host function error: Fixture script invokeByName op expected non-nil result for "
        "runtimeInvalidByName at index 0"
    );
    requireRuntimeRow(
        "runtimeInvokeMalformedArgs",
        "Host function error: Fixture script invoke op requires array args at index 0"
    );
    requireRuntimeRow(
        "runtimeInvokeMalformedExpect",
        "Host function error: Fixture script invoke op unsupported expect value "
        "'unknown_expect' at index 0"
    );
    requireRuntimeRow(
        "runtimeNestedInvokeMalformedStore",
        "Host function error: Fixture script invoke op requires string store at index 0"
    );
    requireRuntimeRow(
        "runtimeNestedInvokeMalformedExpectObject",
        "Host function error: Fixture script invoke op requires supported expect object at index 0"
    );
    requireRuntimeRow(
        "runtimeNestedInvokeByNameMalformedStore",
        "Host function error: Fixture script invokeByName op requires string store at index 0"
    );
    requireRuntimeRow(
        "runtimeNestedInvokeByNameMalformedExpectObject",
        "Host function error: Fixture script invokeByName op requires supported expect object at index 0"
    );
    requireRuntimeRow(
        "runtimeDeepMixedInvokeByNameBadResolverParts",
        "Host function error: Fixture script resolver concat requires array parts"
    );
    requireRuntimeRow(
        "runtimeDeepMixedInvokeByNameUnknownResolverSource",
        "Host function error: Fixture script resolver unknown source 'unknown_resolver'"
    );
    requireRuntimeRow(
        "runtimeDeepMixedConcatBadParts",
        "Host function error: Fixture script resolver concat requires array parts"
    );
    requireRuntimeRow(
        "runtimeDeepMixedCoalesceBadValues",
        "Host function error: Fixture script resolver coalesce requires array values"
    );
    requireRuntimeRow(
        "runtimeDeepMixedEqualsMissingRight",
        "Host function error: Fixture script resolver equals requires left and right"
    );
    requireRuntimeRow(
        "runtimeDeepMixedArgBadIndex",
        "Host function error: Fixture script resolver arg requires integer index"
    );
    requireRuntimeRow(
        "runtimeDeepMixedParamBadName",
        "Host function error: Fixture script resolver param requires string name"
    );
    requireRuntimeRow(
        "runtimeDeepMixedLocalBadName",
        "Host function error: Fixture script resolver local requires string name"
    );
    requireRuntimeRow(
        "runtimeDeepMixedHasArgBadIndex",
        "Host function error: Fixture script resolver hasArg requires integer index",
        "HARD_FAIL"
    );
    requireRuntimeRow(
        "runtimeDeepMixedHasParamBadName",
        "Host function error: Fixture script resolver hasParam requires string name",
        "HARD_FAIL"
    );
    requireRuntimeRow(
        "runtimeDeepMixedArgCountUnexpectedField",
        "Host function error: Fixture script resolver argCount does not accept field 'index'",
        "HARD_FAIL"
    );
    requireRuntimeRow(
        "runtimeDeepMixedArgsUnexpectedField",
        "Host function error: Fixture script resolver args does not accept field 'name'",
        "HARD_FAIL"
    );
    requireRuntimeRow(
        "runtimeDeepMixedParamKeysUnexpectedField",
        "Host function error: Fixture script resolver paramKeys does not accept field 'index'",
        "HARD_FAIL"
    );
}

void verifyLoadPluginDiagnosticRows(
    const DiagnosticRows& diagnostics,
    const std::filesystem::path& missingFixture,
    const std::filesystem::path& emptyNameFixture) {
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_command_shape"}, {"plugin", "MixedChainCommandShapeFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_command_name"}, {"plugin", "MixedChainCommandNameFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_drop_context_flag"},
            {"plugin", "MixedChainDropContextFlagFixture"},
            {"command", "badDropFlag"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_js_entry"},
            {"plugin", "MixedChainEntryTypeFixture"},
            {"command", "badEntry"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_command_description"},
            {"plugin", "MixedChainCommandDescriptionFixture"},
            {"command", "badDescription"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_command_mode"},
            {"plugin", "MixedChainUnsupportedModeFixture"},
            {"command", "badModeValue"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_fixture_open"}, {"plugin", missingFixture.stem().string()}}
    );
    requireDiagnosticRow(diagnostics, {{"operation", "load_plugins_directory_scan"}});

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

    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_duplicate"}, {"plugin", "MixedChainDuplicateFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_fixture_name"}, {"plugin", emptyNameFixture.stem().string()}}
    );
    requireDiagnosticRow(diagnostics, {{"operation", "parse_parameters_name"}});
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "parse_parameters_json"}, {"plugin", "MixedChainParamsFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_dependencies"}, {"plugin", "MixedChainDependencyShapeFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_dependency_entry"}, {"plugin", "MixedChainDependencyEntryFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_parameters"}, {"plugin", "MixedChainParametersShapeFixture"}}
    );
    requireDiagnosticRow(
        diagnostics,
        {{"operation", "load_plugin_commands"}, {"plugin", "MixedChainCommandsShapeFixture"}}
    );
    requireDiagnosticRow(diagnostics, {{"operation", "load_plugin_name"}, {"plugin", ""}});
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_register_command"},
            {"plugin", "MixedChainRegisterCommandFixture"},
            {"command", "dup"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_register_script_fn"},
            {"plugin", "MixedChainRegisterScriptFnFixture"},
            {"command", "__urpg_fail_register_function___scriptCommand"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "load_plugin_quickjs_context"},
            {"plugin", "MixedChain__urpg_fail_context_init__Fixture"},
            {"command", "ctxInitFail"}
        }
    );
    requireDiagnosticRow(
        diagnostics,
        {
            {"operation", "execute_command_quickjs_context_missing"},
            {"plugin", "MixedChainRuntimeFixture"},
            {"command", "runtimeContextMissingViaDrop"},
            {"message", "QuickJS context missing for plugin: MixedChainRuntimeFixture"}
        }
    );
}

void loadCuratedFixturePluginsAndVerifyHappyPaths(PluginManager& pm) {
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
}

void registerDependencyGateProbesAndVerifyBlocking(PluginManager& pm) {
    std::vector<std::string> executedProbePlugins;

    for (const auto& spec : fixtureSpecs()) {
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

    for (const auto& spec : fixtureSpecs()) {
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
}

std::filesystem::path createAndExerciseWeeklyLifecycleFixture(PluginManager& pm) {
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

    return weeklyLifecycleFixture;
}

} // namespace

TEST_CASE(
    "Compat fixtures: combined weekly regression covers curated dependency gating and mixed malformed/runtime chains",
    "[compat][fixtures][failure][weekly]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto& specs = fixtureSpecs();
    loadCuratedFixturePluginsAndVerifyHappyPaths(pm);
    registerDependencyGateProbesAndVerifyBlocking(pm);
    const auto weeklyLifecycleFixture = createAndExerciseWeeklyLifecycleFixture(pm);

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

    verifyOperationCounts(diagnostics, specs.size());
    verifyDependencyGateDiagnosticRows(diagnostics, specs);
    verifyRuntimeDiagnosticRows(diagnostics);
    verifyLoadPluginDiagnosticRows(diagnostics, missingFixture, emptyNameFixture);

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
