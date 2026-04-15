#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"
#include "runtimes/compat_js/window_compat.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

using urpg::Object;
using urpg::Value;
using urpg::compat::BattleAction;
using urpg::compat::BattleActionType;
using urpg::compat::BattleManager;
using urpg::compat::BattlePhase;
using urpg::compat::BattleResult;
using urpg::compat::BattleSubject;
using urpg::compat::BattleSubjectType;
using urpg::compat::DataManager;
using urpg::compat::PluginManager;
using urpg::compat::Window_Base;

struct FixtureSpec {
    std::string pluginName;
    std::string commandName;
    std::string expectedProfile;
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

std::filesystem::path uniqueTempFixturePath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

const std::vector<FixtureSpec>& fixtureSpecs() {
    static const std::vector<FixtureSpec> specs = {
        {"VisuStella_CoreEngine_MZ", "boot", "core"},
        {"VisuStella_MainMenuCore_MZ", "openMenu", "menu_core"},
        {"VisuStella_OptionsCore_MZ", "openOptions", "options_core"},
        {"CGMZ_MenuCommandWindow", "refresh", "command_window"},
        {"CGMZ_Encyclopedia", "openCategory", "encyclopedia"},
        {"EliMZ_Book", "openBook", "book"},
        {"MOG_CharacterMotion_MZ", "startMotion", "character_motion"},
        {"MOG_BattleHud_MZ", "showHud", "battle_hud"},
        {"Galv_QuestLog_MZ", "openQuestLog", "quest_log"},
        {"AltMenuScreen_MZ", "applyLayout", "alt_menu"},
    };
    return specs;
}

void expectScriptResult(const Value& result,
                        const FixtureSpec& spec,
                        int64_t expectedArgCount,
                        const std::string& expectedFirstArg) {
    REQUIRE(std::holds_alternative<Object>(result.v));
    const auto& object = std::get<Object>(result.v);

    const auto pluginIt = object.find("plugin");
    REQUIRE(pluginIt != object.end());
    REQUIRE(std::holds_alternative<std::string>(pluginIt->second.v));
    REQUIRE(std::get<std::string>(pluginIt->second.v) == spec.pluginName);

    const auto commandIt = object.find("command");
    REQUIRE(commandIt != object.end());
    REQUIRE(std::holds_alternative<std::string>(commandIt->second.v));
    REQUIRE(std::get<std::string>(commandIt->second.v) == spec.commandName);

    const auto profileIt = object.find("profile");
    REQUIRE(profileIt != object.end());
    REQUIRE(std::holds_alternative<std::string>(profileIt->second.v));
    REQUIRE(std::get<std::string>(profileIt->second.v) == spec.expectedProfile);

    const auto argCountIt = object.find("argCount");
    REQUIRE(argCountIt != object.end());
    REQUIRE(std::holds_alternative<int64_t>(argCountIt->second.v));
    REQUIRE(std::get<int64_t>(argCountIt->second.v) == expectedArgCount);

    const auto firstArgIt = object.find("firstArg");
    if (firstArgIt != object.end()) {
        REQUIRE(std::holds_alternative<std::string>(firstArgIt->second.v));
        REQUIRE(std::get<std::string>(firstArgIt->second.v) == expectedFirstArg);
    }
}

} // namespace

TEST_CASE("Compat fixtures: plugin JSON fixtures load and execute representative commands", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();

    const auto fixturesRoot = fixtureDir();
    REQUIRE(std::filesystem::exists(fixturesRoot));

    const auto& specs = fixtureSpecs();
    for (const auto& spec : specs) {
        INFO("Fixture: " << spec.pluginName);
        REQUIRE(pm.loadPlugin(fixturePath(spec.pluginName).string()));
    }

    REQUIRE(pm.getLoadedPlugins().size() == specs.size());
    REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    for (const auto& spec : specs) {
        INFO("Command: " << spec.pluginName << "." << spec.commandName);
        Value fixtureArg;
        fixtureArg.v = std::string("fixture_arg");
        const Value result = pm.executeCommand(
            spec.pluginName,
            spec.commandName,
            {fixtureArg}
        );
        expectScriptResult(result, spec, 1, "fixture_arg");
    }

    std::vector<Value> args;
    args.push_back(Value::Int(10));
    args.push_back(Value::Int(20));
    args.push_back(Value::Int(30));

    const Value argCountResult = pm.executeCommand("VisuStella_CoreEngine_MZ", "countArgs", args);
    REQUIRE(std::holds_alternative<int64_t>(argCountResult.v));
    REQUIRE(std::get<int64_t>(argCountResult.v) == 3);

    const Value argCountViaJsResult =
        pm.executeCommand("VisuStella_CoreEngine_MZ", "countArgsViaJs", args);
    REQUIRE(std::holds_alternative<int64_t>(argCountViaJsResult.v));
    REQUIRE(std::get<int64_t>(argCountViaJsResult.v) == 3);

    const Value profile = pm.getParameter("VisuStella_CoreEngine_MZ", "profile");
    REQUIRE(std::holds_alternative<std::string>(profile.v));
    REQUIRE(std::get<std::string>(profile.v) == "core");

    const Value coreDslResult = pm.executeCommand(
        "VisuStella_CoreEngine_MZ",
        "boot",
        {Value::Int(9), Value::Int(12)}
    );
    REQUIRE(std::holds_alternative<Object>(coreDslResult.v));
    const auto& coreDslObject = std::get<Object>(coreDslResult.v);

    const auto hasProfileIt = coreDslObject.find("hasProfileParam");
    REQUIRE(hasProfileIt != coreDslObject.end());
    REQUIRE(std::holds_alternative<bool>(hasProfileIt->second.v));
    REQUIRE(std::get<bool>(hasProfileIt->second.v));

    const auto argsEchoIt = coreDslObject.find("argsEcho");
    REQUIRE(argsEchoIt != coreDslObject.end());
    REQUIRE(std::holds_alternative<urpg::Array>(argsEchoIt->second.v));
    const auto& argsEcho = std::get<urpg::Array>(argsEchoIt->second.v);
    REQUIRE(argsEcho.size() == 2);
    REQUIRE(std::holds_alternative<int64_t>(argsEcho[0].v));
    REQUIRE(std::get<int64_t>(argsEcho[0].v) == 9);
    REQUIRE(std::holds_alternative<int64_t>(argsEcho[1].v));
    REQUIRE(std::get<int64_t>(argsEcho[1].v) == 12);

    const auto paramKeysIt = coreDslObject.find("paramKeys");
    REQUIRE(paramKeysIt != coreDslObject.end());
    REQUIRE(std::holds_alternative<urpg::Array>(paramKeysIt->second.v));
    const auto& paramKeys = std::get<urpg::Array>(paramKeysIt->second.v);
    REQUIRE(paramKeys.size() == 2);
    REQUIRE(std::holds_alternative<std::string>(paramKeys[0].v));
    REQUIRE(std::holds_alternative<std::string>(paramKeys[1].v));
    REQUIRE(std::get<std::string>(paramKeys[0].v) == "profile");
    REQUIRE(std::get<std::string>(paramKeys[1].v) == "supportsWindowBase");

    const Value noArgResult = pm.executeCommand("VisuStella_CoreEngine_MZ", "boot", {});
    expectScriptResult(noArgResult, fixtureSpecs().front(), 0, "none");

    Value argString;
    argString.v = std::string("menu_arg");
    const Value firstArgViaJs =
        pm.executeCommand("VisuStella_MainMenuCore_MZ", "firstArgViaJs", {argString});
    REQUIRE(std::holds_alternative<std::string>(firstArgViaJs.v));
    REQUIRE(std::get<std::string>(firstArgViaJs.v) == "menu_arg");

    const Value optionsConstViaJs =
        pm.executeCommand("VisuStella_OptionsCore_MZ", "optionsConstViaJs", {});
    REQUIRE(std::holds_alternative<std::string>(optionsConstViaJs.v));
    REQUIRE(std::get<std::string>(optionsConstViaJs.v) == "options_runtime_token");

    const Value categoryArgViaJs =
        pm.executeCommand("CGMZ_Encyclopedia", "categoryArgViaJs", {Value::Int(77)});
    REQUIRE(std::holds_alternative<int64_t>(categoryArgViaJs.v));
    REQUIRE(std::get<int64_t>(categoryArgViaJs.v) == 77);

    const Value visibleRowsConstViaJs =
        pm.executeCommand("CGMZ_MenuCommandWindow", "visibleRowsConstViaJs", {});
    REQUIRE(std::holds_alternative<int64_t>(visibleRowsConstViaJs.v));
    REQUIRE(std::get<int64_t>(visibleRowsConstViaJs.v) == 8);

    const Value pageArgViaJs =
        pm.executeCommand("EliMZ_Book", "pageArgViaJs", {Value::Int(12)});
    REQUIRE(std::holds_alternative<int64_t>(pageArgViaJs.v));
    REQUIRE(std::get<int64_t>(pageArgViaJs.v) == 12);

    const Value motionScaleConstViaJs =
        pm.executeCommand("MOG_CharacterMotion_MZ", "motionScaleConstViaJs", {});
    REQUIRE(std::holds_alternative<bool>(motionScaleConstViaJs.v));
    REQUIRE(std::get<bool>(motionScaleConstViaJs.v));

    const Value hudEnabledViaJs =
        pm.executeCommand("MOG_BattleHud_MZ", "hudEnabledViaJs", {});
    REQUIRE(std::holds_alternative<bool>(hudEnabledViaJs.v));
    REQUIRE(std::get<bool>(hudEnabledViaJs.v));

    const Value questArgViaJs = pm.executeCommand(
        "Galv_QuestLog_MZ",
        "questArgViaJs",
        {Value::Int(1), Value::Int(99)}
    );
    REQUIRE(std::holds_alternative<int64_t>(questArgViaJs.v));
    REQUIRE(std::get<int64_t>(questArgViaJs.v) == 99);

    const Value layoutColumnsViaJs =
        pm.executeCommand("AltMenuScreen_MZ", "layoutColumnsViaJs", {});
    REQUIRE(std::holds_alternative<int64_t>(layoutColumnsViaJs.v));
    REQUIRE(std::get<int64_t>(layoutColumnsViaJs.v) == 2);

    pm.unloadAllPlugins();
    REQUIRE(pm.getLoadedPlugins().empty());
}

TEST_CASE("Compat fixtures: directory loader discovers and loads all fixture plugins", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();

    const auto loadedCount = pm.loadPluginsFromDirectory(fixtureDir().string());
    REQUIRE(loadedCount == static_cast<int32_t>(fixtureSpecs().size()));

    for (const auto& spec : fixtureSpecs()) {
        INFO("Loaded fixture plugin: " << spec.pluginName);
        REQUIRE(pm.isPluginLoaded(spec.pluginName));
        REQUIRE(pm.hasCommand(spec.pluginName, spec.commandName));
    }

    auto loadedPlugins = pm.getLoadedPlugins();
    REQUIRE(loadedPlugins.size() == fixtureSpecs().size());

    std::vector<std::string> expectedLoaded;
    expectedLoaded.reserve(fixtureSpecs().size());
    for (const auto& spec : fixtureSpecs()) {
        expectedLoaded.push_back(spec.pluginName);
    }
    std::sort(expectedLoaded.begin(), expectedLoaded.end());

    REQUIRE(loadedPlugins == expectedLoaded);

    pm.unloadAllPlugins();
}

TEST_CASE("Compat fixtures: fixture script DSL supports branching/comparison/contains/logical flows",
          "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto dslFixture = uniqueTempFixturePath("urpg_dsl_fixture");
    writeTextFile(
        dslFixture,
        R"({
  "name": "DslProbeFixture",
  "parameters": {
    "profile": "dsl_profile"
  },
  "commands": [
    {
      "name": "probe",
      "script": [
        {"op": "set", "key": "plugin", "value": {"from": "pluginName"}},
        {"op": "set", "key": "command", "value": {"from": "commandName"}},
        {"op": "set", "key": "missingArgFallback", "value": {"from": "arg", "index": 3, "default": "fallback_arg"}},
        {"op": "set", "key": "missingParamFallback", "value": {"from": "param", "name": "missingParam", "default": "fallback_param"}},
        {"op": "set", "key": "argOrProfile", "value": {"from": "coalesce", "values": [
          {"from": "arg", "index": 7},
          {"from": "param", "name": "profile"}
        ], "default": "none"}},
        {"op": "append", "key": "trace", "value": {"from": "concat", "parts": ["plugin=", {"from": "pluginName"}]}},
        {"op": "append", "key": "trace", "value": {"from": "concat", "parts": ["command=", {"from": "commandName"}]}},
        {"op": "if", "condition": {"from": "equals", "left": {"from": "arg", "index": 1}, "right": 99},
          "then": [
            {"op": "set", "key": "branch", "value": "matched"}
          ],
          "else": [
            {"op": "set", "key": "branch", "value": "not_matched"}
          ]
        },
        {"op": "set", "key": "branchCopy", "value": {"from": "local", "name": "branch"}},
        {"op": "set", "key": "hasArg0", "value": {"from": "hasArg", "index": 0}},
        {"op": "set", "key": "hasArg9", "value": {"from": "hasArg", "index": 9}},
        {"op": "set", "key": "argsLength", "value": {"from": "length", "value": {"from": "args"}}},
        {"op": "set", "key": "profileLength", "value": {"from": "length", "value": {"from": "param", "name": "profile"}}},
        {"op": "set", "key": "traceHasCommand", "value": {"from": "contains", "container": {"from": "local", "name": "trace"}, "value": "command=probe"}},
        {"op": "set", "key": "paramKeysHasProfile", "value": {"from": "contains", "container": {"from": "paramKeys"}, "value": "profile"}},
        {"op": "set", "key": "profileContainsDsl", "value": {"from": "contains", "container": {"from": "param", "name": "profile"}, "value": "dsl"}},
        {"op": "set", "key": "arg1GreaterThanArg0", "value": {"from": "greaterThan", "left": {"from": "arg", "index": 1}, "right": {"from": "arg", "index": 0}}},
        {"op": "set", "key": "arg0LessThanArg1", "value": {"from": "lessThan", "left": {"from": "arg", "index": 0}, "right": {"from": "arg", "index": 1}}},
        {"op": "set", "key": "notEqualsNegative", "value": {"from": "not", "value": {"from": "equals", "left": {"from": "arg", "index": 0}, "right": -1}}},
        {"op": "set", "key": "allChecks", "value": {"from": "all", "values": [
          {"from": "hasArg", "index": 0},
          {"from": "contains", "container": {"from": "param", "name": "profile"}, "value": "dsl"},
          {"from": "greaterThan", "left": {"from": "arg", "index": 1}, "right": {"from": "arg", "index": 0}}
        ]}},
        {"op": "set", "key": "anyChecks", "value": {"from": "any", "values": [
          {"from": "hasArg", "index": 9},
          {"from": "equals", "left": {"from": "arg", "index": 1}, "right": 99}
        ]}},
        {"op": "set", "key": "allInvalidDefault", "value": {"from": "all", "values": {"bad": "shape"}, "default": true}},
        {"op": "set", "key": "anyInvalidDefault", "value": {"from": "any", "values": {"bad": "shape"}, "default": false}},
        {"op": "returnObject"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(dslFixture.string()));

    const Value result = pm.executeCommand(
        "DslProbeFixture",
        "probe",
        {Value::Int(1), Value::Int(99)}
    );
    REQUIRE(std::holds_alternative<Object>(result.v));
    const auto& object = std::get<Object>(result.v);

    REQUIRE(std::get<std::string>(object.at("plugin").v) == "DslProbeFixture");
    REQUIRE(std::get<std::string>(object.at("command").v) == "probe");
    REQUIRE(std::get<std::string>(object.at("missingArgFallback").v) == "fallback_arg");
    REQUIRE(std::get<std::string>(object.at("missingParamFallback").v) == "fallback_param");
    REQUIRE(std::get<std::string>(object.at("argOrProfile").v) == "dsl_profile");
    REQUIRE(std::get<std::string>(object.at("branch").v) == "matched");
    REQUIRE(std::get<std::string>(object.at("branchCopy").v) == "matched");
    REQUIRE(std::holds_alternative<bool>(object.at("hasArg0").v));
    REQUIRE(std::get<bool>(object.at("hasArg0").v));
    REQUIRE(std::holds_alternative<bool>(object.at("hasArg9").v));
    REQUIRE_FALSE(std::get<bool>(object.at("hasArg9").v));
    REQUIRE(std::holds_alternative<int64_t>(object.at("argsLength").v));
    REQUIRE(std::get<int64_t>(object.at("argsLength").v) == 2);
    REQUIRE(std::holds_alternative<int64_t>(object.at("profileLength").v));
    REQUIRE(std::get<int64_t>(object.at("profileLength").v) == 11);
    REQUIRE(std::holds_alternative<bool>(object.at("traceHasCommand").v));
    REQUIRE(std::get<bool>(object.at("traceHasCommand").v));
    REQUIRE(std::holds_alternative<bool>(object.at("paramKeysHasProfile").v));
    REQUIRE(std::get<bool>(object.at("paramKeysHasProfile").v));
    REQUIRE(std::holds_alternative<bool>(object.at("profileContainsDsl").v));
    REQUIRE(std::get<bool>(object.at("profileContainsDsl").v));
    REQUIRE(std::holds_alternative<bool>(object.at("arg1GreaterThanArg0").v));
    REQUIRE(std::get<bool>(object.at("arg1GreaterThanArg0").v));
    REQUIRE(std::holds_alternative<bool>(object.at("arg0LessThanArg1").v));
    REQUIRE(std::get<bool>(object.at("arg0LessThanArg1").v));
    REQUIRE(std::holds_alternative<bool>(object.at("notEqualsNegative").v));
    REQUIRE(std::get<bool>(object.at("notEqualsNegative").v));
    REQUIRE(std::holds_alternative<bool>(object.at("allChecks").v));
    REQUIRE(std::get<bool>(object.at("allChecks").v));
    REQUIRE(std::holds_alternative<bool>(object.at("anyChecks").v));
    REQUIRE(std::get<bool>(object.at("anyChecks").v));
    REQUIRE(std::holds_alternative<bool>(object.at("allInvalidDefault").v));
    REQUIRE(std::get<bool>(object.at("allInvalidDefault").v));
    REQUIRE(std::holds_alternative<bool>(object.at("anyInvalidDefault").v));
    REQUIRE_FALSE(std::get<bool>(object.at("anyInvalidDefault").v));

    REQUIRE(std::holds_alternative<urpg::Array>(object.at("trace").v));
    const auto& trace = std::get<urpg::Array>(object.at("trace").v);
    REQUIRE(trace.size() == 2);
    REQUIRE(std::holds_alternative<std::string>(trace[0].v));
    REQUIRE(std::holds_alternative<std::string>(trace[1].v));
    REQUIRE(std::get<std::string>(trace[0].v) == "plugin=DslProbeFixture");
    REQUIRE(std::get<std::string>(trace[1].v) == "command=probe");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(dslFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script DSL supports invoke command chaining across fixtures",
          "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));

    const auto invokeFixture = uniqueTempFixturePath("urpg_dsl_invoke_fixture");
    writeTextFile(
        invokeFixture,
        R"({
  "name": "DslInvokeFixture",
  "commands": [
    {
      "name": "chain",
      "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "store": "coreBoot", "expect": "non_nil"},
        {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "mainMenuOpen", "expect": "non_nil"},
        {"op": "returnObject"}
      ]
    }
  ]
})"
    );

    REQUIRE(pm.loadPlugin(invokeFixture.string()));

    const Value result = pm.executeCommand("DslInvokeFixture", "chain", {});
    REQUIRE(std::holds_alternative<Object>(result.v));
    const auto& object = std::get<Object>(result.v);

    REQUIRE(std::holds_alternative<Object>(object.at("coreBoot").v));
    const auto& coreBoot = std::get<Object>(object.at("coreBoot").v);
    REQUIRE(std::holds_alternative<std::string>(coreBoot.at("plugin").v));
    REQUIRE(std::get<std::string>(coreBoot.at("plugin").v) == "VisuStella_CoreEngine_MZ");

    REQUIRE(std::holds_alternative<Object>(object.at("mainMenuOpen").v));
    const auto& mainMenuOpen = std::get<Object>(object.at("mainMenuOpen").v);
    REQUIRE(std::holds_alternative<std::string>(mainMenuOpen.at("plugin").v));
    REQUIRE(std::get<std::string>(mainMenuOpen.at("plugin").v) == "VisuStella_MainMenuCore_MZ");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(invokeFixture, ec);
}

TEST_CASE("Compat fixtures: nested invokeByName resolver flows support deeper mixed branches",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));

        const auto nestedFixture = uniqueTempFixturePath("urpg_dsl_nested_invoke_by_name_fixture");
        writeTextFile(
                nestedFixture,
                R"({
    "name": "DslNestedInvokeByNameFixture",
    "parameters": {
        "defaultRoute": "menu"
    },
    "commands": [
        {
            "name": "route",
            "script": [
                {
                    "op": "if",
                    "condition": {
                        "from": "equals",
                        "left": {
                            "from": "coalesce",
                            "values": [
                                {"from": "arg", "index": 0},
                                {"from": "param", "name": "defaultRoute"}
                            ]
                        },
                        "right": "menu"
                    },
                    "then": [
                        {
                            "op": "if",
                            "condition": {
                                "from": "equals",
                                "left": {"from": "arg", "index": 1, "default": "primary"},
                                "right": "primary"
                            },
                            "then": [
                                {
                                    "op": "invokeByName",
                                    "name": {"from": "concat", "parts": ["VisuStella_MainMenuCore_MZ", "_openMenu"]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ],
                            "else": [
                                {
                                    "op": "invokeByName",
                                    "name": {
                                        "from": "coalesce",
                                        "values": [
                                            {"from": "local", "name": "missingName"},
                                            {"from": "concat", "parts": ["VisuStella_CoreEngine_MZ", "_boot"]}
                                        ]
                                    },
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ]
                        }
                    ],
                    "else": [
                        {
                            "op": "if",
                            "condition": {
                                "from": "equals",
                                "left": {"from": "arg", "index": 1, "default": "menu"},
                                "right": "menu"
                            },
                            "then": [
                                {
                                    "op": "invokeByName",
                                    "name": {"from": "concat", "parts": ["VisuStella_MainMenuCore_MZ", "_openMenu"]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ],
                            "else": [
                                {
                                    "op": "invokeByName",
                                    "name": {
                                        "from": "coalesce",
                                        "values": [
                                            {"from": "local", "name": "missingName"},
                                            {"from": "concat", "parts": ["VisuStella_CoreEngine_MZ", "_boot"]}
                                        ]
                                    },
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ]
                        }
                    ]
                },
                {
                    "op": "set",
                    "key": "decision",
                    "value": {
                        "from": "concat",
                        "parts": [
                            {
                                "from": "coalesce",
                                "values": [
                                    {"from": "arg", "index": 0},
                                    {"from": "param", "name": "defaultRoute"}
                                ]
                            },
                            "/",
                            {"from": "arg", "index": 1, "default": "primary"}
                        ]
                    }
                },
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(nestedFixture.string()));

        const Value defaultRouteResult = pm.executeCommand("DslNestedInvokeByNameFixture", "route", {});
        REQUIRE(std::holds_alternative<Object>(defaultRouteResult.v));
        const auto& defaultRouteObject = std::get<Object>(defaultRouteResult.v);
        REQUIRE(std::holds_alternative<Object>(defaultRouteObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(defaultRouteObject.at("selected").v).at("plugin").v) ==
                "VisuStella_MainMenuCore_MZ"
        );
        REQUIRE(std::get<std::string>(defaultRouteObject.at("decision").v) == "menu/primary");

        Value thenElseArg0;
        thenElseArg0.v = std::string("menu");
        Value thenElseArg1;
        thenElseArg1.v = std::string("secondary");
        const Value thenElseResult =
                pm.executeCommand("DslNestedInvokeByNameFixture", "route", {thenElseArg0, thenElseArg1});
        REQUIRE(std::holds_alternative<Object>(thenElseResult.v));
        const auto& thenElseObject = std::get<Object>(thenElseResult.v);
        REQUIRE(std::holds_alternative<Object>(thenElseObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(thenElseObject.at("selected").v).at("plugin").v) ==
                "VisuStella_CoreEngine_MZ"
        );
        REQUIRE(std::get<std::string>(thenElseObject.at("decision").v) == "menu/secondary");

        Value elseThenArg0;
        elseThenArg0.v = std::string("fallback");
        Value elseThenArg1;
        elseThenArg1.v = std::string("menu");
        const Value elseThenResult =
                pm.executeCommand("DslNestedInvokeByNameFixture", "route", {elseThenArg0, elseThenArg1});
        REQUIRE(std::holds_alternative<Object>(elseThenResult.v));
        const auto& elseThenObject = std::get<Object>(elseThenResult.v);
        REQUIRE(std::holds_alternative<Object>(elseThenObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(elseThenObject.at("selected").v).at("plugin").v) ==
                "VisuStella_MainMenuCore_MZ"
        );
        REQUIRE(std::get<std::string>(elseThenObject.at("decision").v) == "fallback/menu");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(nestedFixture, ec);
}

TEST_CASE("Compat fixtures: nested invoke resolver flows support deeper mixed branches",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));

        const auto nestedFixture = uniqueTempFixturePath("urpg_dsl_nested_invoke_fixture");
        writeTextFile(
                nestedFixture,
                R"({
    "name": "DslNestedInvokeFixture",
    "parameters": {
        "defaultPlugin": "VisuStella_MainMenuCore_MZ",
        "defaultCommand": "openMenu"
    },
    "commands": [
        {
            "name": "route",
            "script": [
                {
                    "op": "if",
                    "condition": {
                        "from": "equals",
                        "left": {"from": "arg", "index": 0, "default": "menu"},
                        "right": "menu"
                    },
                    "then": [
                        {
                            "op": "if",
                            "condition": {
                                "from": "equals",
                                "left": {"from": "arg", "index": 1, "default": "primary"},
                                "right": "primary"
                            },
                            "then": [
                                {
                                    "op": "invoke",
                                    "plugin": {"from": "coalesce", "values": [{"from": "local", "name": "missingPlugin", "default": null}, {"from": "param", "name": "defaultPlugin"}]},
                                    "command": {"from": "coalesce", "values": [{"from": "local", "name": "missingCommand", "default": null}, {"from": "param", "name": "defaultCommand"}]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ],
                            "else": [
                                {
                                    "op": "invoke",
                                    "plugin": {"from": "concat", "parts": ["VisuStella_CoreEngine", "_MZ"]},
                                    "command": {"from": "concat", "parts": ["bo", "ot"]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ]
                        }
                    ],
                    "else": [
                        {
                            "op": "if",
                            "condition": {
                                "from": "equals",
                                "left": {"from": "arg", "index": 1, "default": "menu"},
                                "right": "menu"
                            },
                            "then": [
                                {
                                    "op": "invoke",
                                    "plugin": {"from": "coalesce", "values": [{"from": "param", "name": "defaultPlugin"}]},
                                    "command": {"from": "coalesce", "values": [{"from": "param", "name": "defaultCommand"}]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ],
                            "else": [
                                {
                                    "op": "invoke",
                                    "plugin": {"from": "concat", "parts": ["VisuStella_CoreEngine", "_MZ"]},
                                    "command": {"from": "concat", "parts": ["bo", "ot"]},
                                    "store": "selected",
                                    "expect": "non_nil"
                                }
                            ]
                        }
                    ]
                },
                {
                    "op": "set",
                    "key": "decision",
                    "value": {
                        "from": "concat",
                        "parts": [
                            {"from": "arg", "index": 0, "default": "menu"},
                            "/",
                            {"from": "arg", "index": 1, "default": "primary"}
                        ]
                    }
                },
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(nestedFixture.string()));

        const Value defaultRouteResult = pm.executeCommand("DslNestedInvokeFixture", "route", {});
        REQUIRE(std::holds_alternative<Object>(defaultRouteResult.v));
        const auto& defaultRouteObject = std::get<Object>(defaultRouteResult.v);
        REQUIRE(std::holds_alternative<Object>(defaultRouteObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(defaultRouteObject.at("selected").v).at("plugin").v) ==
                "VisuStella_MainMenuCore_MZ"
        );
        REQUIRE(std::get<std::string>(defaultRouteObject.at("decision").v) == "menu/primary");

        Value thenElseArg0;
        thenElseArg0.v = std::string("menu");
        Value thenElseArg1;
        thenElseArg1.v = std::string("secondary");
        const Value thenElseResult =
                pm.executeCommand("DslNestedInvokeFixture", "route", {thenElseArg0, thenElseArg1});
        REQUIRE(std::holds_alternative<Object>(thenElseResult.v));
        const auto& thenElseObject = std::get<Object>(thenElseResult.v);
        REQUIRE(std::holds_alternative<Object>(thenElseObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(thenElseObject.at("selected").v).at("plugin").v) ==
                "VisuStella_CoreEngine_MZ"
        );
        REQUIRE(std::get<std::string>(thenElseObject.at("decision").v) == "menu/secondary");

        Value elseThenArg0;
        elseThenArg0.v = std::string("fallback");
        Value elseThenArg1;
        elseThenArg1.v = std::string("menu");
        const Value elseThenResult =
                pm.executeCommand("DslNestedInvokeFixture", "route", {elseThenArg0, elseThenArg1});
        REQUIRE(std::holds_alternative<Object>(elseThenResult.v));
        const auto& elseThenObject = std::get<Object>(elseThenResult.v);
        REQUIRE(std::holds_alternative<Object>(elseThenObject.at("selected").v));
        REQUIRE(
                std::get<std::string>(std::get<Object>(elseThenObject.at("selected").v).at("plugin").v) ==
                "VisuStella_MainMenuCore_MZ"
        );
        REQUIRE(std::get<std::string>(elseThenObject.at("decision").v) == "fallback/menu");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(nestedFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script DSL supports rich invoke expectations",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));

        const auto expectFixture = uniqueTempFixturePath("urpg_dsl_expect_fixture");
        writeTextFile(
                expectFixture,
                R"({
    "name": "DslExpectFixture",
    "commands": [
        {
            "name": "emitNil",
            "result": null
        },
        {
            "name": "chain",
            "script": [
                {
                    "op": "invoke",
                    "plugin": "VisuStella_CoreEngine_MZ",
                    "command": "boot",
                    "store": "coreBoot",
                    "expect": {
                        "equals": {
                            "plugin": "VisuStella_CoreEngine_MZ",
                            "command": "boot",
                            "profile": "core",
                            "argCount": 0,
                            "firstArg": "none",
                            "hasProfileParam": true,
                            "argsEcho": [],
                            "paramKeys": ["profile", "supportsWindowBase"]
                        }
                    }
                },
                {
                    "op": "invoke",
                    "plugin": "VisuStella_CoreEngine_MZ",
                    "command": "countArgs",
                    "args": [1, 2],
                    "store": "argCount",
                    "expect": {"equals": 2}
                },
                {
                    "op": "invoke",
                    "plugin": "MOG_BattleHud_MZ",
                    "command": "hudEnabledViaJs",
                    "store": "hudEnabled",
                    "expect": "truthy"
                },
                {
                    "op": "invoke",
                    "command": "emitNil",
                    "store": "nilValue",
                    "expect": "nil"
                },
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(expectFixture.string()));

        const Value result = pm.executeCommand("DslExpectFixture", "chain", {});
        REQUIRE(std::holds_alternative<Object>(result.v));
        const auto& object = std::get<Object>(result.v);

        expectScriptResult(object.at("coreBoot"), fixtureSpecs().front(), 0, "none");
        REQUIRE(std::holds_alternative<int64_t>(object.at("argCount").v));
        REQUIRE(std::get<int64_t>(object.at("argCount").v) == 2);
        REQUIRE(std::holds_alternative<bool>(object.at("hudEnabled").v));
        REQUIRE(std::get<bool>(object.at("hudEnabled").v));
        REQUIRE(std::holds_alternative<std::monostate>(object.at("nilValue").v));

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(expectFixture, ec);
}

TEST_CASE("Compat fixtures: curated menu-stack scenarios preserve plugin-specific behavior",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));
        REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));

        const auto scenarioFixture = uniqueTempFixturePath("urpg_curated_menu_stack_fixture");
        writeTextFile(
                scenarioFixture,
                R"({
    "name": "CuratedMenuScenarioFixture",
    "parameters": {
        "defaultRoute": "options"
    },
    "commands": [
        {
            "name": "run",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "menu_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "menu", "expect": "non_nil"},
                {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "options"},
                    "then": [
                        {"op": "invoke", "plugin": "VisuStella_OptionsCore_MZ", "command": "openOptions", "store": "routeResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "supportResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "VisuStella_OptionsCore_MZ", "command": "optionsConstViaJs", "store": "routeToken", "expect": {"equals": "options_runtime_token"}}
                    ],
                    "else": [
                        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "layout"},
                            "then": [
                                {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "applyLayout", "store": "routeResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "supportResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "layoutColumnsViaJs", "store": "routeToken", "expect": {"equals": 2}}
                            ],
                            "else": [
                                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "supportResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "hudEnabledViaJs", "store": "routeToken", "expect": "truthy"}
                            ]
                        }
                    ]
                },
                {"op": "set", "key": "summary", "value": {"from": "concat", "parts": [
                    {"from": "local", "name": "route"},
                    ":",
                    {"from": "param", "name": "defaultRoute"},
                    ":",
                    {"from": "local", "name": "routeToken"}
                ]}},
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(scenarioFixture.string()));

        const Value defaultResult = pm.executeCommand("CuratedMenuScenarioFixture", "run", {});
        REQUIRE(std::holds_alternative<Object>(defaultResult.v));
        const auto& defaultObject = std::get<Object>(defaultResult.v);
        REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("boot").v).at("plugin").v) ==
                        "VisuStella_CoreEngine_MZ");
        REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("menu").v).at("profile").v) ==
                        "menu_core");
        REQUIRE(std::get<std::string>(defaultObject.at("route").v) == "options");
        REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("routeResult").v).at("profile").v) ==
                        "options_core");
        REQUIRE(std::get<int64_t>(std::get<Object>(defaultObject.at("supportResult").v).at("visibleRows").v) == 8);
        REQUIRE(std::get<std::string>(defaultObject.at("routeToken").v) == "options_runtime_token");
        REQUIRE(std::get<std::string>(defaultObject.at("summary").v) == "options:options:options_runtime_token");

        Value layoutSeed;
        layoutSeed.v = std::string("layout_boot");
        Value layoutRoute;
        layoutRoute.v = std::string("layout");
        const Value layoutResult = pm.executeCommand(
                "CuratedMenuScenarioFixture",
                "run",
                {layoutSeed, layoutRoute}
        );
        REQUIRE(std::holds_alternative<Object>(layoutResult.v));
        const auto& layoutObject = std::get<Object>(layoutResult.v);
        REQUIRE(std::get<std::string>(std::get<Object>(layoutObject.at("boot").v).at("firstArg").v) ==
                        "layout_boot");
        REQUIRE(std::get<std::string>(layoutObject.at("route").v) == "layout");
        REQUIRE(std::get<std::string>(std::get<Object>(layoutObject.at("routeResult").v).at("layout").v) ==
                        "horizontal");
        REQUIRE(std::get<int64_t>(layoutObject.at("routeToken").v) == 2);
        REQUIRE(std::get<std::string>(layoutObject.at("summary").v) == "layout:options:2");

        Value battleSeed;
        battleSeed.v = std::string("battle_boot");
        Value battleRoute;
        battleRoute.v = std::string("battle");
        const Value battleResult = pm.executeCommand(
                "CuratedMenuScenarioFixture",
                "run",
                {battleSeed, battleRoute}
        );
        REQUIRE(std::holds_alternative<Object>(battleResult.v));
        const auto& battleObject = std::get<Object>(battleResult.v);
        REQUIRE(std::get<std::string>(battleObject.at("route").v) == "battle");
        REQUIRE(std::get<std::string>(std::get<Object>(battleObject.at("routeResult").v).at("profile").v) ==
                        "battle_hud");
        REQUIRE(std::get<int64_t>(std::get<Object>(battleObject.at("routeResult").v).at("hudSlots").v) == 4);
        REQUIRE(std::get<bool>(battleObject.at("routeToken").v));
        REQUIRE(std::get<std::string>(battleObject.at("summary").v) == "battle:options:true");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(scenarioFixture, ec);
}

TEST_CASE("Compat fixtures: curated codex scenarios preserve content-plugin behavior",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_Encyclopedia").string()));
        REQUIRE(pm.loadPlugin(fixturePath("EliMZ_Book").string()));
        REQUIRE(pm.loadPlugin(fixturePath("Galv_QuestLog_MZ").string()));

        const auto scenarioFixture = uniqueTempFixturePath("urpg_curated_codex_fixture");
        writeTextFile(
                scenarioFixture,
                R"({
    "name": "CuratedCodexScenarioFixture",
    "parameters": {
        "defaultTopic": "quests"
    },
    "commands": [
        {
            "name": "explore",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "codex_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "set", "key": "topic", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultTopic"}]}} ,
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "topic"}, "right": "encyclopedia"},
                    "then": [
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "openCategory", "args": [{"from": "arg", "index": 2, "default": "bestiary"}], "store": "topicResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "categoryArgViaJs", "args": [{"from": "arg", "index": 2, "default": 5}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 2, "default": 5}}}
                    ],
                    "else": [
                        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "topic"}, "right": "book"},
                            "then": [
                                {"op": "invokeByName", "name": "EliMZ_Book_openBook", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "topicResult", "expect": "non_nil"},
                                {"op": "invokeByName", "name": "EliMZ_Book_pageArgViaJs", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 2, "default": 12}}}
                            ],
                            "else": [
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "openQuestLog", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "topicResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "questArgViaJs", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 3, "default": 77}}}
                            ]
                        }
                    ]
                },
                {"op": "set", "key": "summary", "value": {"from": "concat", "parts": [
                    {"from": "local", "name": "topic"},
                    ":",
                    {"from": "local", "name": "topicToken"},
                    ":",
                    {"from": "local", "name": "topic"}
                ]}},
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(scenarioFixture.string()));

        const Value defaultResult = pm.executeCommand("CuratedCodexScenarioFixture", "explore", {});
        REQUIRE(std::holds_alternative<Object>(defaultResult.v));
        const auto& defaultObject = std::get<Object>(defaultResult.v);
        REQUIRE(std::get<std::string>(defaultObject.at("topic").v) == "quests");
        REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("topicResult").v).at("profile").v) ==
                        "quest_log");
        REQUIRE(std::get<int64_t>(std::get<Object>(defaultObject.at("topicResult").v).at("questCount").v) == 10);
        REQUIRE(std::get<int64_t>(defaultObject.at("topicToken").v) == 77);
        REQUIRE(std::get<std::string>(defaultObject.at("summary").v) == "quests:77:quests");

        Value encyclTopic;
        encyclTopic.v = std::string("ency_boot");
        Value encyclRoute;
        encyclRoute.v = std::string("encyclopedia");
        Value encyclCategory;
        encyclCategory.v = std::string("bestiary");
        const Value encyclopediaResult = pm.executeCommand(
                "CuratedCodexScenarioFixture",
                "explore",
                {encyclTopic, encyclRoute, encyclCategory}
        );
        REQUIRE(std::holds_alternative<Object>(encyclopediaResult.v));
        const auto& encyclopediaObject = std::get<Object>(encyclopediaResult.v);
        REQUIRE(std::get<std::string>(std::get<Object>(encyclopediaObject.at("boot").v).at("firstArg").v) ==
                        "ency_boot");
        REQUIRE(std::get<std::string>(encyclopediaObject.at("topic").v) == "encyclopedia");
        REQUIRE(std::get<std::string>(std::get<Object>(encyclopediaObject.at("topicResult").v).at("profile").v) ==
                        "encyclopedia");
        REQUIRE(std::get<int64_t>(std::get<Object>(encyclopediaObject.at("topicResult").v).at("categoryCount").v) == 3);
        REQUIRE(std::get<std::string>(encyclopediaObject.at("topicToken").v) == "bestiary");
        REQUIRE(std::get<std::string>(encyclopediaObject.at("summary").v) == "encyclopedia:bestiary:encyclopedia");

        Value bookBoot;
        bookBoot.v = std::string("book_boot");
        Value bookRoute;
        bookRoute.v = std::string("book");
        const Value bookResult = pm.executeCommand(
                "CuratedCodexScenarioFixture",
                "explore",
                {bookBoot, bookRoute, Value::Int(42)}
        );
        REQUIRE(std::holds_alternative<Object>(bookResult.v));
        const auto& bookObject = std::get<Object>(bookResult.v);
        REQUIRE(std::get<std::string>(bookObject.at("topic").v) == "book");
        REQUIRE(std::get<std::string>(std::get<Object>(bookObject.at("topicResult").v).at("profile").v) ==
                        "book");
        REQUIRE(std::get<int64_t>(std::get<Object>(bookObject.at("topicResult").v).at("pages").v) == 12);
        REQUIRE(std::get<int64_t>(bookObject.at("topicToken").v) == 42);
        REQUIRE(std::get<std::string>(bookObject.at("summary").v) == "book:42:book");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(scenarioFixture, ec);
}

TEST_CASE("Compat fixtures: curated codex scenarios survive plugin reload",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_Encyclopedia").string()));
        REQUIRE(pm.loadPlugin(fixturePath("EliMZ_Book").string()));
        REQUIRE(pm.loadPlugin(fixturePath("Galv_QuestLog_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_codex_reload_fixture");
        writeTextFile(
                reloadFixture,
                R"({
    "name": "CuratedCodexReloadFixture",
    "parameters": {
        "defaultTopic": "quests"
    },
    "commands": [
        {
            "name": "explore",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "codex_reload_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "set", "key": "topic", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultTopic"}]}} ,
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "topic"}, "right": "encyclopedia"},
                    "then": [
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "openCategory", "args": [{"from": "arg", "index": 2, "default": "bestiary"}], "store": "topicResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "categoryArgViaJs", "args": [{"from": "arg", "index": 2, "default": "bestiary"}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 2, "default": "bestiary"}}}
                    ],
                    "else": [
                        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "topic"}, "right": "book"},
                            "then": [
                                {"op": "invokeByName", "name": "EliMZ_Book_openBook", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "topicResult", "expect": "non_nil"},
                                {"op": "invokeByName", "name": "EliMZ_Book_pageArgViaJs", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 2, "default": 12}}}
                            ],
                            "else": [
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "openQuestLog", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "topicResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "questArgViaJs", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "topicToken", "expect": {"equals": {"from": "arg", "index": 3, "default": 77}}}
                            ]
                        }
                    ]
                },
                {"op": "set", "key": "summary", "value": {"from": "concat", "parts": [
                    {"from": "local", "name": "topic"},
                    ":",
                    {"from": "local", "name": "topicToken"},
                    ":",
                    {"from": "local", "name": "topic"}
                ]}},
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyQuestRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("topic").v) == "quests");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("topicResult").v).at("profile").v) == "quest_log");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("topicResult").v).at("questCount").v) == 10);
                REQUIRE(std::get<int64_t>(object.at("topicToken").v) == 77);
                REQUIRE(std::get<std::string>(object.at("summary").v) == "quests:77:quests");
        };

        auto verifyEncyclopediaRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("topic").v) == "encyclopedia");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("topicResult").v).at("profile").v) == "encyclopedia");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("topicResult").v).at("categoryCount").v) == 3);
                REQUIRE(std::get<std::string>(object.at("topicToken").v) == "bestiary");
                REQUIRE(std::get<std::string>(object.at("summary").v) == "encyclopedia:bestiary:encyclopedia");
        };

        auto verifyBookRoute = [&](const Value& value, const std::string& expectedBoot, int64_t expectedPage) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("topic").v) == "book");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("topicResult").v).at("profile").v) == "book");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("topicResult").v).at("pages").v) == 12);
                REQUIRE(std::get<int64_t>(object.at("topicToken").v) == expectedPage);
                REQUIRE(std::get<std::string>(object.at("summary").v) == "book:" + std::to_string(expectedPage) + ":book");
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_codex");
        const Value beforeReloadQuest =
                pm.executeCommand("CuratedCodexReloadFixture", "explore", {beforeReloadBoot});
        verifyQuestRoute(beforeReloadQuest, "before_reload_codex");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("CGMZ_Encyclopedia"));
        REQUIRE(pm.reloadPlugin("EliMZ_Book"));
        REQUIRE(pm.reloadPlugin("Galv_QuestLog_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedCodexReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedCodexReloadFixture", "explore"));
        REQUIRE(pm.hasCommand("CGMZ_Encyclopedia", "openCategory"));
        REQUIRE(pm.hasCommand("EliMZ_Book", "openBook"));
        REQUIRE(pm.hasCommand("Galv_QuestLog_MZ", "openQuestLog"));

        Value afterReloadQuestBoot;
        afterReloadQuestBoot.v = std::string("after_reload_quest");
        const Value afterReloadQuest =
                pm.executeCommandByName("CuratedCodexReloadFixture_explore", {afterReloadQuestBoot});
        verifyQuestRoute(afterReloadQuest, "after_reload_quest");

        Value encyclBoot;
        encyclBoot.v = std::string("after_reload_ency");
        Value encyclRoute;
        encyclRoute.v = std::string("encyclopedia");
        Value encyclCategory;
        encyclCategory.v = std::string("bestiary");
        const Value afterReloadEncyclopedia =
                pm.executeCommand("CuratedCodexReloadFixture", "explore", {encyclBoot, encyclRoute, encyclCategory});
        verifyEncyclopediaRoute(afterReloadEncyclopedia, "after_reload_ency");

        Value bookBoot;
        bookBoot.v = std::string("after_reload_book");
        Value bookRoute;
        bookRoute.v = std::string("book");
        const Value afterReloadBook =
                pm.executeCommandByName("CuratedCodexReloadFixture_explore", {bookBoot, bookRoute, Value::Int(42)});
        verifyBookRoute(afterReloadBook, "after_reload_book", 42);

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated library-dashboard scenarios survive plugin reload",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_Encyclopedia").string()));
        REQUIRE(pm.loadPlugin(fixturePath("EliMZ_Book").string()));
        REQUIRE(pm.loadPlugin(fixturePath("Galv_QuestLog_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_library_dashboard_reload_fixture");
        writeTextFile(
                reloadFixture,
                R"({
    "name": "CuratedLibraryDashboardReloadFixture",
    "parameters": {
        "defaultRoute": "quests"
    },
    "commands": [
        {
            "name": "open",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "library_reload_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "encyclopedia"},
                    "then": [
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "openCategory", "args": [{"from": "arg", "index": 2, "default": "bestiary"}], "store": "routeResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "CGMZ_Encyclopedia", "command": "categoryArgViaJs", "args": [{"from": "arg", "index": 2, "default": "bestiary"}], "store": "routeToken", "expect": {"equals": {"from": "arg", "index": 2, "default": "bestiary"}}}
                    ],
                    "else": [
                        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "book"},
                            "then": [
                                {"op": "invokeByName", "name": "EliMZ_Book_openBook", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "routeResult", "expect": "non_nil"},
                                {"op": "invokeByName", "name": "EliMZ_Book_pageArgViaJs", "args": [{"from": "arg", "index": 2, "default": 12}], "store": "routeToken", "expect": {"equals": {"from": "arg", "index": 2, "default": 12}}}
                            ],
                            "else": [
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "openQuestLog", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "routeResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "Galv_QuestLog_MZ", "command": "questArgViaJs", "args": [{"from": "arg", "index": 2, "default": 1}, {"from": "arg", "index": 3, "default": 77}], "store": "routeToken", "expect": {"equals": {"from": "arg", "index": 3, "default": 77}}}
                            ]
                        }
                    ]
                },
                {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "dashboard", "expect": "non_nil"},
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyQuestRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "quests");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "quest_log");
                REQUIRE(std::get<int64_t>(object.at("routeToken").v) == 77);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("dashboard").v).at("profile").v) == "command_window");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("dashboard").v).at("visibleRows").v) == 8);
        };

        auto verifyEncyclopediaRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "encyclopedia");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "encyclopedia");
                REQUIRE(std::get<std::string>(object.at("routeToken").v) == "bestiary");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("dashboard").v).at("profile").v) == "command_window");
        };

        auto verifyBookRoute = [&](const Value& value, const std::string& expectedBoot, int64_t expectedPage) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "book");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "book");
                REQUIRE(std::get<int64_t>(object.at("routeToken").v) == expectedPage);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("dashboard").v).at("profile").v) == "command_window");
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_library");
        const Value beforeReloadQuest =
                pm.executeCommand("CuratedLibraryDashboardReloadFixture", "open", {beforeReloadBoot});
        verifyQuestRoute(beforeReloadQuest, "before_reload_library");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("CGMZ_MenuCommandWindow"));
        REQUIRE(pm.reloadPlugin("CGMZ_Encyclopedia"));
        REQUIRE(pm.reloadPlugin("EliMZ_Book"));
        REQUIRE(pm.reloadPlugin("Galv_QuestLog_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedLibraryDashboardReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedLibraryDashboardReloadFixture", "open"));
        REQUIRE(pm.hasCommand("CGMZ_MenuCommandWindow", "refresh"));
        REQUIRE(pm.hasCommand("CGMZ_Encyclopedia", "openCategory"));
        REQUIRE(pm.hasCommand("EliMZ_Book", "openBook"));
        REQUIRE(pm.hasCommand("Galv_QuestLog_MZ", "openQuestLog"));

        Value encyclBoot;
        encyclBoot.v = std::string("after_reload_library_ency");
        Value encyclRoute;
        encyclRoute.v = std::string("encyclopedia");
        Value encyclCategory;
        encyclCategory.v = std::string("bestiary");
        const Value afterReloadEncyclopedia =
                pm.executeCommandByName("CuratedLibraryDashboardReloadFixture_open", {encyclBoot, encyclRoute, encyclCategory});
        verifyEncyclopediaRoute(afterReloadEncyclopedia, "after_reload_library_ency");

        Value bookBoot;
        bookBoot.v = std::string("after_reload_library_book");
        Value bookRoute;
        bookRoute.v = std::string("book");
        const Value afterReloadBook =
                pm.executeCommand("CuratedLibraryDashboardReloadFixture", "open", {bookBoot, bookRoute, Value::Int(42)});
        verifyBookRoute(afterReloadBook, "after_reload_library_book", 42);

        Value questBoot;
        questBoot.v = std::string("after_reload_library_quest");
        const Value afterReloadQuest =
                pm.executeCommandByName("CuratedLibraryDashboardReloadFixture_open", {questBoot});
        verifyQuestRoute(afterReloadQuest, "after_reload_library_quest");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
}

    TEST_CASE("Compat fixtures: curated message-text scenarios survive plugin reload",
                "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        DataManager& data = DataManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        data.setupNewGame();
        data.setVariable(2, 777);

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_message_text_reload_fixture");
        writeTextFile(
            reloadFixture,
            R"({
        "name": "CuratedMessageTextReloadFixture",
        "parameters": {
        "defaultRoute": "speaker",
        "defaultSpeaker": "Alicia",
        "defaultBody": "\\C[2]HP\\I[5]\\V[2]\\G",
        "defaultNarration": "The door creaks open...\nFootsteps echo.",
        "defaultSystem": "\\C[16]System\\C[0]: Autosave complete"
        },
        "commands": [
        {
            "name": "render",
            "script": [
            {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "message_reload_boot"}], "store": "boot", "expect": "non_nil"},
            {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
            {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "narration"},
                "then": [
                {"op": "set", "key": "speaker", "value": ""},
                {"op": "set", "key": "faceActorId", "value": 0},
                {"op": "set", "key": "body", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultNarration"}}},
                {"op": "set", "key": "layoutMode", "value": "narration"},
                {"op": "set", "key": "tone", "value": "neutral"}
                ],
                "else": [
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "system"},
                    "then": [
                    {"op": "set", "key": "speaker", "value": "System"},
                    {"op": "set", "key": "faceActorId", "value": 0},
                    {"op": "set", "key": "body", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultSystem"}}},
                    {"op": "set", "key": "layoutMode", "value": "system"},
                    {"op": "set", "key": "tone", "value": "system"}
                    ],
                    "else": [
                    {"op": "set", "key": "speaker", "value": {"from": "arg", "index": 3, "default": {"from": "param", "name": "defaultSpeaker"}}},
                    {"op": "set", "key": "faceActorId", "value": {"from": "arg", "index": 4, "default": 3}},
                    {"op": "set", "key": "body", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultBody"}}},
                    {"op": "set", "key": "layoutMode", "value": "speaker"},
                    {"op": "set", "key": "tone", "value": "portrait"}
                    ]
                }
                ]
            },
            {"op": "set", "key": "routeToken", "value": {"from": "concat", "parts": [{"from": "local", "name": "route"}, ":", {"from": "local", "name": "layoutMode"}, ":", {"from": "local", "name": "tone"}]}} ,
            {"op": "returnObject"}
            ]
        }
        ]
    })"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto requireObjectString = [](const Object& object, const std::string& key) {
            const auto it = object.find(key);
            REQUIRE(it != object.end());
            REQUIRE(std::holds_alternative<std::string>(it->second.v));
            return std::get<std::string>(it->second.v);
        };

        auto requireObjectInt = [](const Object& object, const std::string& key) {
            const auto it = object.find(key);
            REQUIRE(it != object.end());
            REQUIRE(std::holds_alternative<int64_t>(it->second.v));
            return std::get<int64_t>(it->second.v);
        };

        auto requireObjectRecord = [](const Object& object, const std::string& key) -> const Object& {
            const auto it = object.find(key);
            REQUIRE(it != object.end());
            REQUIRE(std::holds_alternative<Object>(it->second.v));
            return std::get<Object>(it->second.v);
        };

        auto verifySpeakerRoute = [&](const Value& value, const std::string& expectedBoot) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(requireObjectString(requireObjectRecord(object, "boot"), "firstArg") == expectedBoot);
            REQUIRE(requireObjectString(object, "route") == "speaker");
            REQUIRE(requireObjectString(object, "layoutMode") == "speaker");
            REQUIRE(requireObjectString(object, "tone") == "portrait");
            REQUIRE(requireObjectString(object, "speaker") == "Alicia");
            REQUIRE(requireObjectInt(object, "faceActorId") == 3);
            REQUIRE(requireObjectString(object, "routeToken") == "speaker:speaker:portrait");
            REQUIRE(requireObjectString(object, "body") == "\\C[2]HP\\I[5]\\V[2]\\G");

            Window_Base window(Window_Base::CreateParams{});
            const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
            const uint32_t drawIconBefore = Window_Base::getMethodCallCount("drawIcon");
            const uint32_t colorBefore = Window_Base::getMethodCallCount("changeTextColor");

            window.drawActorFace(static_cast<int32_t>(requireObjectInt(object, "faceActorId")), 8, 8, 144, 144);
            const auto faceInfo = window.getLastFaceDraw();
            REQUIRE(faceInfo.has_value());
            REQUIRE(faceInfo->actorId == 3);

            window.drawText(requireObjectString(object, "speaker"), 160, 0, 120, "left");
            window.drawTextEx(requireObjectString(object, "body"), 160, 36);

            REQUIRE(Window_Base::getMethodCallCount("drawText") >= drawTextBefore + 3);
            REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 2);
            REQUIRE(Window_Base::getMethodCallCount("changeTextColor") == colorBefore + 1);
            REQUIRE(window.textWidth(requireObjectString(object, "body")) > window.textWidth("HP"));
            REQUIRE(window.textSize(requireObjectString(object, "body")).height == window.lineHeight());
        };

        auto verifyNarrationRoute = [&](const Value& value, const std::string& expectedBoot) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(requireObjectString(requireObjectRecord(object, "boot"), "firstArg") == expectedBoot);
            REQUIRE(requireObjectString(object, "route") == "narration");
            REQUIRE(requireObjectString(object, "layoutMode") == "narration");
            REQUIRE(requireObjectString(object, "tone") == "neutral");
            REQUIRE(requireObjectString(object, "speaker").empty());
            REQUIRE(requireObjectInt(object, "faceActorId") == 0);
            REQUIRE(requireObjectString(object, "routeToken") == "narration:narration:neutral");
            REQUIRE(requireObjectString(object, "body") == "The door creaks open...\nFootsteps echo.");

            Window_Base window(Window_Base::CreateParams{});
            const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
            window.drawTextEx(requireObjectString(object, "body"), 0, 0);
            REQUIRE_FALSE(window.getLastFaceDraw().has_value());
            REQUIRE(Window_Base::getMethodCallCount("drawText") >= drawTextBefore + 2);
            REQUIRE(window.textSize(requireObjectString(object, "body")).height == window.lineHeight() * 2);
        };

        auto verifySystemRoute = [&](const Value& value, const std::string& expectedBoot) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(requireObjectString(requireObjectRecord(object, "boot"), "firstArg") == expectedBoot);
            REQUIRE(requireObjectString(object, "route") == "system");
            REQUIRE(requireObjectString(object, "layoutMode") == "system");
            REQUIRE(requireObjectString(object, "tone") == "system");
            REQUIRE(requireObjectString(object, "speaker") == "System");
            REQUIRE(requireObjectInt(object, "faceActorId") == 0);
            REQUIRE(requireObjectString(object, "routeToken") == "system:system:system");

            Window_Base window(Window_Base::CreateParams{});
            const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
            const uint32_t colorBefore = Window_Base::getMethodCallCount("changeTextColor");
            window.drawTextEx(requireObjectString(object, "body"), 0, 0);
            REQUIRE(Window_Base::getMethodCallCount("drawText") >= drawTextBefore + 2);
            REQUIRE(Window_Base::getMethodCallCount("changeTextColor") == colorBefore + 2);
            REQUIRE(window.textSize(requireObjectString(object, "body")).height == window.lineHeight());
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_message");
        const Value beforeReloadSpeaker =
            pm.executeCommand("CuratedMessageTextReloadFixture", "render", {beforeReloadBoot});
        verifySpeakerRoute(beforeReloadSpeaker, "before_reload_message");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedMessageTextReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedMessageTextReloadFixture", "render"));

        Value afterReloadNarrationBoot;
        afterReloadNarrationBoot.v = std::string("after_reload_narration");
        Value narrationRoute;
        narrationRoute.v = std::string("narration");
        const Value afterReloadNarration =
            pm.executeCommandByName("CuratedMessageTextReloadFixture_render", {afterReloadNarrationBoot, narrationRoute});
        verifyNarrationRoute(afterReloadNarration, "after_reload_narration");

        Value afterReloadSystemBoot;
        afterReloadSystemBoot.v = std::string("after_reload_system");
        Value systemRoute;
        systemRoute.v = std::string("system");
        const Value afterReloadSystem =
            pm.executeCommand("CuratedMessageTextReloadFixture", "render", {afterReloadSystemBoot, systemRoute});
        verifySystemRoute(afterReloadSystem, "after_reload_system");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
    }

    TEST_CASE("Compat fixtures: curated save-data scenarios survive plugin reload",
                "[compat][fixtures]") {
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

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_save_data_reload_fixture");
        writeTextFile(
            reloadFixture,
            R"({
        "name": "CuratedSaveDataReloadFixture",
        "parameters": {
        "defaultRoute": "slot",
        "defaultToken": "party"
        },
        "commands": [
        {
            "name": "open",
            "script": [
            {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "save_reload_boot"}], "store": "boot", "expect": "non_nil"},
            {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "menu", "expect": "non_nil"},
            {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "dashboard", "expect": "non_nil"},
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

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        data.setGold(450);
        data.gainItem(2, 7);
        data.setVariable(4, 88);
        data.setPlayerPosition(9, 10, 11);
        data.setPlayerDirection(6);
        REQUIRE(data.saveGame(1));

        Value saveTab;
        saveTab.v = std::string("party");
        REQUIRE(data.setSaveHeaderExtension(1, "ui.tab", saveTab));
        auto saveTabExtension = data.getSaveHeaderExtension(1, "ui.tab");
        REQUIRE(saveTabExtension.has_value());
        REQUIRE(std::holds_alternative<std::string>(saveTabExtension->v));
        REQUIRE(std::get<std::string>(saveTabExtension->v) == "party");

        auto slotHeader = data.getSaveHeader(1);
        REQUIRE(slotHeader.has_value());
        REQUIRE(slotHeader->mapId == 9);
        REQUIRE(slotHeader->playerX == 10);
        REQUIRE(slotHeader->playerY == 11);
        REQUIRE_FALSE(slotHeader->isAutosave);

        auto verifySlotRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedTab) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
            REQUIRE(std::get<std::string>(object.at("route").v) == "slot");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == expectedTab);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("dashboard").v).at("profile").v) == "command_window");
            REQUIRE(std::get<int64_t>(std::get<Object>(object.at("dashboard").v).at("visibleRows").v) == 8);
        };

        auto verifyAutosaveRoute = [&](const Value& value, const std::string& expectedBoot) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
            REQUIRE(std::get<std::string>(object.at("route").v) == "autosave");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == "autosave");
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("dashboard").v).at("profile").v) == "command_window");
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_save_slot");
        Value beforeReloadRoute;
        beforeReloadRoute.v = std::string("slot");
        const Value beforeReloadSlot =
            pm.executeCommand("CuratedSaveDataReloadFixture", "open", {beforeReloadBoot, beforeReloadRoute, saveTab});
        verifySlotRoute(beforeReloadSlot, "before_reload_save_slot", "party");

        data.setAutosaveEnabled(true);
        REQUIRE(data.isAutosaveEnabled());
        data.setVariable(8, 144);
        REQUIRE(data.saveAutosave());
        auto autosaveHeader = data.getSaveHeader(0);
        REQUIRE(autosaveHeader.has_value());
        REQUIRE(autosaveHeader->isAutosave);

        Value autosaveBoot;
        autosaveBoot.v = std::string("before_reload_autosave");
        Value autosaveRoute;
        autosaveRoute.v = std::string("autosave");
        Value autosaveToken;
        autosaveToken.v = std::string("autosave");
        const Value beforeReloadAutosave =
            pm.executeCommandByName("CuratedSaveDataReloadFixture_open", {autosaveBoot, autosaveRoute, autosaveToken});
        verifyAutosaveRoute(beforeReloadAutosave, "before_reload_autosave");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("VisuStella_MainMenuCore_MZ"));
        REQUIRE(pm.reloadPlugin("CGMZ_MenuCommandWindow"));
        REQUIRE(pm.reloadPlugin("CuratedSaveDataReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedSaveDataReloadFixture", "open"));
        REQUIRE(pm.hasCommand("VisuStella_MainMenuCore_MZ", "openMenu"));
        REQUIRE(pm.hasCommand("CGMZ_MenuCommandWindow", "refresh"));

        data.setGold(0);
        data.loseItem(2, 7);
        data.setVariable(4, 0);
        data.setPlayerPosition(1, 0, 0);
        data.setPlayerDirection(2);
        REQUIRE(data.loadGame(1));
        REQUIRE(data.getGold() == 450);
        REQUIRE(data.getItemCount(2) == 7);
        REQUIRE(data.getVariable(4) == 88);
        REQUIRE(data.getPlayerMapId() == 9);
        REQUIRE(data.getPlayerX() == 10);
        REQUIRE(data.getPlayerY() == 11);
        REQUIRE(data.getPlayerDirection() == 6);

        auto reloadedTabExtension = data.getSaveHeaderExtension(1, "ui.tab");
        REQUIRE(reloadedTabExtension.has_value());
        REQUIRE(std::holds_alternative<std::string>(reloadedTabExtension->v));
        REQUIRE(std::get<std::string>(reloadedTabExtension->v) == "party");

        Value afterReloadBoot;
        afterReloadBoot.v = std::string("after_reload_save_slot");
        const Value afterReloadSlot =
            pm.executeCommandByName("CuratedSaveDataReloadFixture_open", {afterReloadBoot, beforeReloadRoute, saveTab});
        verifySlotRoute(afterReloadSlot, "after_reload_save_slot", "party");

        REQUIRE(data.loadAutosave());
        REQUIRE(data.getVariable(8) == 144);
        auto reloadedAutosaveHeader = data.getSaveHeader(0);
        REQUIRE(reloadedAutosaveHeader.has_value());
        REQUIRE(reloadedAutosaveHeader->isAutosave);

        Value afterReloadAutosaveBoot;
        afterReloadAutosaveBoot.v = std::string("after_reload_autosave");
        const Value afterReloadAutosave =
            pm.executeCommand("CuratedSaveDataReloadFixture", "open", {afterReloadAutosaveBoot, autosaveRoute, autosaveToken});
        verifyAutosaveRoute(afterReloadAutosave, "after_reload_autosave");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        data.deleteSaveFile(0);
        data.deleteSaveFile(1);
        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
    }

TEST_CASE("Compat fixtures: curated menu-presentation scenarios survive plugin reload",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));
        REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_menu_presentation_reload_fixture");
        writeTextFile(
                reloadFixture,
                R"({
    "name": "CuratedMenuPresentationReloadFixture",
    "parameters": {
        "defaultRoute": "layout"
    },
    "commands": [
        {
            "name": "run",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "menu_presentation_reload_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "menu", "expect": "non_nil"},
                {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
                {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "commandWindow", "expect": "non_nil"},
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "hud"},
                    "then": [
                        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "hudEnabledViaJs", "store": "routeToken", "expect": "truthy"}
                    ],
                    "else": [
                        {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "applyLayout", "store": "routeResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "layoutColumnsViaJs", "store": "routeToken", "expect": {"equals": 2}}
                    ]
                },
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyLayoutRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("menu").v).at("columnCount").v) == 2);
                REQUIRE(std::get<std::string>(object.at("route").v) == "layout");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "alt_menu");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("layout").v) == "horizontal");
                REQUIRE(std::get<int64_t>(object.at("routeToken").v) == 2);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) == "command_window");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("commandWindow").v).at("visibleRows").v) == 8);
        };

        auto verifyHudRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
                REQUIRE(std::get<std::string>(object.at("route").v) == "hud");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "battle_hud");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("hudSlots").v) == 4);
                REQUIRE(std::get<bool>(object.at("routeToken").v));
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) == "command_window");
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_menu_layout");
        const Value beforeReloadLayout =
                pm.executeCommand("CuratedMenuPresentationReloadFixture", "run", {beforeReloadBoot});
        verifyLayoutRoute(beforeReloadLayout, "before_reload_menu_layout");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("VisuStella_MainMenuCore_MZ"));
        REQUIRE(pm.reloadPlugin("CGMZ_MenuCommandWindow"));
        REQUIRE(pm.reloadPlugin("AltMenuScreen_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedMenuPresentationReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedMenuPresentationReloadFixture", "run"));
        REQUIRE(pm.hasCommand("VisuStella_MainMenuCore_MZ", "openMenu"));
        REQUIRE(pm.hasCommand("CGMZ_MenuCommandWindow", "refresh"));
        REQUIRE(pm.hasCommand("AltMenuScreen_MZ", "applyLayout"));
        REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));

        Value layoutBoot;
        layoutBoot.v = std::string("after_reload_menu_layout");
        const Value afterReloadLayout =
                pm.executeCommandByName("CuratedMenuPresentationReloadFixture_run", {layoutBoot});
        verifyLayoutRoute(afterReloadLayout, "after_reload_menu_layout");

        Value hudBoot;
        hudBoot.v = std::string("after_reload_menu_hud");
        Value hudRoute;
        hudRoute.v = std::string("hud");
        const Value afterReloadHud =
                pm.executeCommand("CuratedMenuPresentationReloadFixture", "run", {hudBoot, hudRoute});
        verifyHudRoute(afterReloadHud, "after_reload_menu_hud");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated presentation scenarios survive plugin reload",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_presentation_reload_fixture");
        writeTextFile(
                reloadFixture,
                R"({
    "name": "CuratedPresentationReloadFixture",
    "parameters": {
        "defaultRoute": "motion"
    },
    "commands": [
        {
            "name": "run",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "presentation_reload_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
                {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": "idle"}},
                {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "layout"},
                    "then": [
                        {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "applyLayout", "store": "routeResult", "expect": "non_nil"},
                        {"op": "invoke", "plugin": "AltMenuScreen_MZ", "command": "layoutColumnsViaJs", "store": "routeToken", "expect": {"equals": 2}}
                    ],
                    "else": [
                        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "hud"},
                            "then": [
                                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "hudEnabledViaJs", "store": "routeToken", "expect": "truthy"}
                            ],
                            "else": [
                                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "routeResult", "expect": "non_nil"},
                                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "motionScaleConstViaJs", "store": "routeToken", "expect": "truthy"}
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

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyLayoutRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "layout");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "alt_menu");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("layout").v) == "horizontal");
                REQUIRE(std::get<int64_t>(object.at("routeToken").v) == 2);
        };

        auto verifyHudRoute = [&](const Value& value, const std::string& expectedBoot) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "hud");
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "battle_hud");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("hudSlots").v) == 4);
                REQUIRE(std::get<bool>(object.at("routeToken").v));
        };

        auto verifyMotionRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotionName) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
                REQUIRE(std::get<std::string>(object.at("route").v) == "motion");
                REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotionName);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "character_motion");
                REQUIRE(std::get<bool>(std::get<Object>(object.at("routeResult").v).at("supportsScale").v));
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("argCount").v) == 1);
                REQUIRE(std::get<bool>(object.at("routeToken").v));
        };

        Value beforeReloadBoot;
        beforeReloadBoot.v = std::string("before_reload_motion");
        Value beforeReloadRoute;
        beforeReloadRoute.v = std::string("motion");
        Value beforeReloadMotion;
        beforeReloadMotion.v = std::string("dash");
        const Value beforeReload =
                pm.executeCommand("CuratedPresentationReloadFixture", "run", {beforeReloadBoot, beforeReloadRoute, beforeReloadMotion});
        verifyMotionRoute(beforeReload, "before_reload_motion", "dash");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("AltMenuScreen_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedPresentationReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedPresentationReloadFixture", "run"));
        REQUIRE(pm.hasCommand("AltMenuScreen_MZ", "applyLayout"));
        REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
        REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

        Value layoutBoot;
        layoutBoot.v = std::string("after_reload_layout");
        Value layoutRoute;
        layoutRoute.v = std::string("layout");
        const Value afterReloadLayout =
                pm.executeCommandByName("CuratedPresentationReloadFixture_run", {layoutBoot, layoutRoute});
        verifyLayoutRoute(afterReloadLayout, "after_reload_layout");

        Value hudBoot;
        hudBoot.v = std::string("after_reload_hud");
        Value hudRoute;
        hudRoute.v = std::string("hud");
        const Value afterReloadHud =
                pm.executeCommand("CuratedPresentationReloadFixture", "run", {hudBoot, hudRoute});
        verifyHudRoute(afterReloadHud, "after_reload_hud");

        Value motionBoot;
        motionBoot.v = std::string("after_reload_motion");
        Value motionRoute;
        motionRoute.v = std::string("motion");
        Value motionName;
        motionName.v = std::string("float");
        const Value afterReloadMotion =
                pm.executeCommandByName("CuratedPresentationReloadFixture_run", {motionBoot, motionRoute, motionName});
        verifyMotionRoute(afterReloadMotion, "after_reload_motion", "float");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
}

    TEST_CASE("Compat fixtures: curated battle-flow scenarios survive plugin reload",
                "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_flow_reload_fixture");
        writeTextFile(
            reloadFixture,
            R"({
        "name": "CuratedBattleFlowReloadFixture",
        "parameters": {
        "defaultRoute": "action",
        "defaultMotion": "slash"
        },
        "commands": [
        {
            "name": "engage",
            "script": [
            {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_flow_boot"}], "store": "boot", "expect": "non_nil"},
            {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
            {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
            {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "escape"},
                "then": [
                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "routeResult", "expect": "non_nil"},
                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "supportResult", "expect": "non_nil"},
                {"op": "set", "key": "routeToken", "value": "escape:attempt"}
                ],
                "else": [
                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult", "expect": "non_nil"},
                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "supportResult", "expect": "non_nil"},
                {"op": "set", "key": "routeToken", "value": "action:resolve"}
                ]
            },
            {"op": "returnObject"}
            ]
        }
        ]
    })"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyActionRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(object.at("route").v) == "action");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == "action:resolve");
                REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "battle_hud");
            REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("hudSlots").v) == 4);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("supportResult").v).at("profile").v) == "character_motion");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("supportResult").v).at("argCount").v) == 1);
                REQUIRE(std::get<bool>(std::get<Object>(object.at("supportResult").v).at("supportsScale").v));

            BattleManager battle;
            int actionStartCount = 0;
            int actionEndCount = 0;
            int damageCount = 0;
            int healCount = 0;
            battle.registerHook(BattleManager::HookPoint::ON_ACTION_START, "battle_anchor", [&actionStartCount](const std::vector<Value>&) {
                ++actionStartCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_ACTION_END, "battle_anchor", [&actionEndCount](const std::vector<Value>&) {
                ++actionEndCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_anchor", [&damageCount](const std::vector<Value>&) {
                ++damageCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_HEAL, "battle_anchor", [&healCount](const std::vector<Value>&) {
                ++healCount;
                return Value::Nil();
            });

            battle.setup(11, true, false);
            battle.startBattle();
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
            battle.startTurn();
            REQUIRE(battle.getPhase() == BattlePhase::TURN);

            BattleSubject actor;
            actor.type = BattleSubjectType::ACTOR;
            actor.index = 0;
            actor.hp = 50;
            actor.mhp = 100;
            actor.actionSpeed = 18;

            BattleSubject enemy;
            enemy.type = BattleSubjectType::ENEMY;
            enemy.index = 0;
            enemy.hp = 90;
            enemy.mhp = 90;

            battle.queueAction(&actor, BattleActionType::WAIT);
            BattleAction* action = battle.getNextAction();
            REQUIRE(action != nullptr);
            REQUIRE(action->type == BattleActionType::WAIT);
            battle.processAction(action);
            REQUIRE(actionStartCount == 1);
            REQUIRE(actionEndCount == 1);

            battle.applyDamage(&enemy, 15);
            battle.applyHeal(&actor, 20);
            REQUIRE(enemy.hp == 75);
            REQUIRE(actor.hp == 70);
            REQUIRE(damageCount == 1);
            REQUIRE(healCount == 1);

            battle.endTurn();
            REQUIRE(battle.getTurnCount() == 1);
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
        };

        auto verifyEscapeRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(object.at("route").v) == "escape");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == "escape:attempt");
                REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "character_motion");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("argCount").v) == 1);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("supportResult").v).at("profile").v) == "battle_hud");

            BattleManager battle;
            int escapeHookCount = 0;
            battle.registerHook(BattleManager::HookPoint::ON_ESCAPE, "battle_anchor", [&escapeHookCount](const std::vector<Value>&) {
                ++escapeHookCount;
                return Value::Nil();
            });

            battle.setup(17, true, false);
            battle.startBattle();
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
            bool escaped = false;
            int attempts = 0;
            while (attempts < 6 && battle.canEscape()) {
                ++attempts;
                escaped = battle.processEscape();
                if (escaped) {
                    break;
                }
            }

            REQUIRE(escaped);
            REQUIRE(attempts >= 1);
            REQUIRE(escapeHookCount == attempts + 1);
            REQUIRE(battle.getResult() == BattleResult::ESCAPE);
            REQUIRE(battle.getPhase() == BattlePhase::NONE);
        };

        Value beforeActionBoot;
        beforeActionBoot.v = std::string("before_reload_battle_action");
        const Value beforeAction =
            pm.executeCommand("CuratedBattleFlowReloadFixture", "engage", {beforeActionBoot});
        verifyActionRoute(beforeAction, "before_reload_battle_action", "slash");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedBattleFlowReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedBattleFlowReloadFixture", "engage"));
        REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
        REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

        Value afterActionBoot;
        afterActionBoot.v = std::string("after_reload_battle_action");
        const Value afterAction =
            pm.executeCommandByName("CuratedBattleFlowReloadFixture_engage", {afterActionBoot});
        verifyActionRoute(afterAction, "after_reload_battle_action", "slash");

        Value escapeBoot;
        escapeBoot.v = std::string("after_reload_battle_escape");
        Value escapeRoute;
        escapeRoute.v = std::string("escape");
        Value retreatMotion;
        retreatMotion.v = std::string("retreat");
        const Value afterEscape =
            pm.executeCommand("CuratedBattleFlowReloadFixture", "engage", {escapeBoot, escapeRoute, retreatMotion});
        verifyEscapeRoute(afterEscape, "after_reload_battle_escape", "retreat");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
    }

TEST_CASE("Compat fixtures: curated battle outcome-status scenarios survive plugin reload",
            "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_outcome_reload_fixture");
    writeTextFile(
        reloadFixture,
        R"({
    "name": "CuratedBattleOutcomeReloadFixture",
    "parameters": {
    "defaultRoute": "victory",
    "defaultMotion": "triumph"
    },
    "commands": [
    {
        "name": "resolve",
        "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_outcome_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
        {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "hud", "expect": "non_nil"},
        {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "motion", "expect": "non_nil"},
        {"op": "set", "key": "routeToken", "value": {"from": "concat", "parts": ["outcome:", {"from": "local", "name": "route"}]}} ,
        {"op": "returnObject"}
        ]
    }
    ]
})"
    );

    REQUIRE(pm.loadPlugin(reloadFixture.string()));

    auto verifyVictoryRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == "victory");
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == "outcome:victory");
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int stateAddedCount = 0;
        int stateRemovedCount = 0;
        int victoryCount = 0;
        std::vector<std::string> damageOrder;

        battle.registerHook(BattleManager::HookPoint::ON_STATE_ADDED, "battle_outcome_anchor", [&stateAddedCount](const std::vector<Value>&) {
            ++stateAddedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_STATE_REMOVED, "battle_outcome_anchor", [&stateRemovedCount](const std::vector<Value>&) {
            ++stateRemovedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_VICTORY, "battle_outcome_anchor", [&victoryCount](const std::vector<Value>&) {
            ++victoryCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_outcome_anchor", [&damageOrder](const std::vector<Value>& args) {
            damageOrder.push_back(std::to_string(std::get<int64_t>(args[0].v)) + ":" + std::to_string(std::get<int64_t>(args[1].v)));
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 40;
        actor.mhp = 50;

        BattleSubject enemy;
        enemy.type = BattleSubjectType::ENEMY;
        enemy.index = 0;
        enemy.hp = 12;
        enemy.mhp = 12;

        battle.setup(19, true, false);
        battle.addActorSubject(actor);
        battle.addEnemySubject(enemy);
        BattleSubject* seededActor = battle.getActor(0);
        BattleSubject* seededEnemy = battle.getEnemy(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(seededEnemy != nullptr);
        REQUIRE(battle.addState(seededActor, 21, 1, 6, 0));
        REQUIRE(battle.addState(seededEnemy, 31, 1, -12, 0));
        REQUIRE(battle.addBuff(seededActor, 6, 2, 1));

        battle.startBattle();
        battle.startTurn();
        battle.endTurn();

        REQUIRE(seededActor->hp == 46);
        REQUIRE(seededEnemy->hp == 0);
        REQUIRE_FALSE(battle.hasState(seededActor, 21));
        REQUIRE_FALSE(battle.hasState(seededEnemy, 31));
        REQUIRE(battle.getModifierStage(seededActor, 6) == 1);
        REQUIRE(stateAddedCount == 2);
        REQUIRE(stateRemovedCount == 2);
        REQUIRE(damageOrder.size() == 1);
        REQUIRE(damageOrder[0] == "1:0");
        REQUIRE(victoryCount == 1);
        REQUIRE(battle.getResult() == BattleResult::WIN);
        REQUIRE(battle.getPhase() == BattlePhase::NONE);
    };

    auto verifyDefeatRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == "defeat");
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == "outcome:defeat");
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int stateAddedCount = 0;
        int stateRemovedCount = 0;
        int defeatCount = 0;
        int actorDeathCount = 0;

        battle.registerHook(BattleManager::HookPoint::ON_STATE_ADDED, "battle_outcome_anchor", [&stateAddedCount](const std::vector<Value>&) {
            ++stateAddedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_STATE_REMOVED, "battle_outcome_anchor", [&stateRemovedCount](const std::vector<Value>&) {
            ++stateRemovedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DEFEAT, "battle_outcome_anchor", [&defeatCount](const std::vector<Value>&) {
            ++defeatCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_ACTOR_DEATH, "battle_outcome_anchor", [&actorDeathCount](const std::vector<Value>&) {
            ++actorDeathCount;
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 9;
        actor.mhp = 20;

        battle.setup(23, true, true);
        battle.addActorSubject(actor);
        BattleSubject* seededActor = battle.getActor(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(battle.addState(seededActor, 41, 1, -12, 0));
        REQUIRE(battle.addDebuff(seededActor, 2, 1, 1));

        battle.startBattle();
        battle.startTurn();
        battle.endTurn();

        REQUIRE(seededActor->hp == 0);
        REQUIRE_FALSE(battle.hasState(seededActor, 41));
        REQUIRE(battle.getModifierStage(seededActor, 2) == 0);
        REQUIRE(stateAddedCount == 1);
        REQUIRE(stateRemovedCount == 1);
        REQUIRE(actorDeathCount == 1);
        REQUIRE(defeatCount == 1);
        REQUIRE(battle.getResult() == BattleResult::DEFEAT);
        REQUIRE(battle.getPhase() == BattlePhase::NONE);
    };

    Value beforeReloadBoot;
    beforeReloadBoot.v = std::string("before_reload_battle_victory");
    const Value beforeReloadVictory =
        pm.executeCommand("CuratedBattleOutcomeReloadFixture", "resolve", {beforeReloadBoot});
    verifyVictoryRoute(beforeReloadVictory, "before_reload_battle_victory", "triumph");

    REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
    REQUIRE(pm.reloadPlugin("CuratedBattleOutcomeReloadFixture"));

    REQUIRE(pm.hasCommand("CuratedBattleOutcomeReloadFixture", "resolve"));
    REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
    REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

    Value afterReloadVictoryBoot;
    afterReloadVictoryBoot.v = std::string("after_reload_battle_victory");
    const Value afterReloadVictory =
        pm.executeCommandByName("CuratedBattleOutcomeReloadFixture_resolve", {afterReloadVictoryBoot});
    verifyVictoryRoute(afterReloadVictory, "after_reload_battle_victory", "triumph");

    Value defeatBoot;
    defeatBoot.v = std::string("after_reload_battle_defeat");
    Value defeatRoute;
    defeatRoute.v = std::string("defeat");
    Value defeatMotion;
    defeatMotion.v = std::string("collapse");
    const Value afterReloadDefeat =
        pm.executeCommand("CuratedBattleOutcomeReloadFixture", "resolve", {defeatBoot, defeatRoute, defeatMotion});
    verifyDefeatRoute(afterReloadDefeat, "after_reload_battle_defeat", "collapse");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated battle tactical-routing scenarios survive plugin reload",
          "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_tactical_reload_fixture");
    writeTextFile(
        reloadFixture,
        R"({
    "name": "CuratedBattleTacticalReloadFixture",
    "parameters": {
    "defaultRoute": "tactical",
    "defaultMotion": "feint"
    },
    "commands": [
    {
        "name": "execute",
        "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_tactical_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}},
        {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "hud", "expect": "non_nil"},
        {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "motion", "expect": "non_nil"},
        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "forced"},
            "then": [{"op": "set", "key": "routeToken", "value": "forced:queue"}],
            "else": [{"op": "set", "key": "routeToken", "value": "tactical:queue"}]
        },
        {"op": "returnObject"}
        ]
    }
    ]
})"
    );

    REQUIRE(pm.loadPlugin(reloadFixture.string()));

    auto verifyTacticalRoute = [&](const Value& value,
                                   const std::string& expectedBoot,
                                   const std::string& expectedRoute,
                                   const std::string& expectedToken,
                                   const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == expectedRoute);
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == expectedToken);
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int actionStartCount = 0;
        int actionEndCount = 0;
        int damageCount = 0;
        battle.registerHook(BattleManager::HookPoint::ON_ACTION_START, "battle_tactical_anchor", [&actionStartCount](const std::vector<Value>&) {
            ++actionStartCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_ACTION_END, "battle_tactical_anchor", [&actionEndCount](const std::vector<Value>&) {
            ++actionEndCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_tactical_anchor", [&damageCount](const std::vector<Value>&) {
            ++damageCount;
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 120;
        actor.mhp = 120;
        actor.actionSpeed = 16;

        BattleSubject enemy;
        enemy.type = BattleSubjectType::ENEMY;
        enemy.index = 0;
        enemy.hp = 100;
        enemy.mhp = 100;
        enemy.actionSpeed = 8;

        battle.setup(31, true, false);
        battle.addActorSubject(actor);
        battle.addEnemySubject(enemy);
        BattleSubject* seededActor = battle.getActor(0);
        BattleSubject* seededEnemy = battle.getEnemy(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(seededEnemy != nullptr);
        REQUIRE(battle.addBuff(seededActor, 2, 2, 2));
        REQUIRE(battle.addDebuff(seededEnemy, 3, 2, 1));

        battle.startBattle();
        battle.startTurn();
        REQUIRE(battle.getPhase() == BattlePhase::TURN);

        battle.setActorAction(0, BattleActionType::ATTACK, -1);
        battle.forceAction(0, BattleSubjectType::ACTOR, BattleActionType::ATTACK, -1);
        battle.sortActionsBySpeed();

        int processedActions = 0;
        while (BattleAction* action = battle.getNextAction()) {
            battle.processAction(action);
            ++processedActions;
        }
        REQUIRE(processedActions == 2);
        REQUIRE(actionStartCount == 2);
        REQUIRE(actionEndCount == 2);
        REQUIRE(damageCount == 2);
        REQUIRE(seededEnemy->hp < 100);
        REQUIRE(battle.getModifierStage(seededActor, 2) == 2);
        REQUIRE(battle.getModifierStage(seededEnemy, 3) == -1);

        REQUIRE(battle.calculateExp() == 10);
        REQUIRE(battle.calculateGold() == 5);

        auto computeDrops = []() {
            BattleManager dropsBattle;
            dropsBattle.setup(37, true, false);
            for (int i = 0; i < 3; ++i) {
                BattleSubject enemyDrop;
                enemyDrop.type = BattleSubjectType::ENEMY;
                enemyDrop.index = i;
                enemyDrop.hp = 30;
                enemyDrop.mhp = 30;
                dropsBattle.addEnemySubject(enemyDrop);
            }
            return dropsBattle.calculateDrops();
        };
        REQUIRE(computeDrops() == computeDrops());

        battle.endTurn();
        REQUIRE(battle.getTurnCount() == 1);
        REQUIRE(battle.getPhase() == BattlePhase::INPUT);
    };

    Value beforeReloadBoot;
    beforeReloadBoot.v = std::string("before_reload_battle_tactical");
    const Value beforeReload =
        pm.executeCommand("CuratedBattleTacticalReloadFixture", "execute", {beforeReloadBoot});
    verifyTacticalRoute(beforeReload,
                        "before_reload_battle_tactical",
                        "tactical",
                        "tactical:queue",
                        "feint");

    REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
    REQUIRE(pm.reloadPlugin("CuratedBattleTacticalReloadFixture"));

    REQUIRE(pm.hasCommand("CuratedBattleTacticalReloadFixture", "execute"));
    REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
    REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

    Value afterReloadBoot;
    afterReloadBoot.v = std::string("after_reload_battle_tactical");
    Value forcedRoute;
    forcedRoute.v = std::string("forced");
    Value forcedMotion;
    forcedMotion.v = std::string("parry");
    const Value afterReload =
        pm.executeCommandByName("CuratedBattleTacticalReloadFixture_execute",
                                {afterReloadBoot, forcedRoute, forcedMotion});
    verifyTacticalRoute(afterReload,
                        "after_reload_battle_tactical",
                        "forced",
                        "forced:queue",
                        "parry");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated menu-stack scenarios survive plugin reload",
                    "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_menu_reload_fixture");
        writeTextFile(
                reloadFixture,
                R"({
    "name": "CuratedMenuReloadFixture",
    "parameters": {
        "defaultRoute": "options"
    },
    "commands": [
        {
            "name": "run",
            "script": [
                {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "reload_boot"}], "store": "boot", "expect": "non_nil"},
                {"op": "invokeByName", "name": "VisuStella_MainMenuCore_MZ_openMenu", "store": "menu", "expect": "non_nil"},
                {"op": "invoke", "plugin": "VisuStella_OptionsCore_MZ", "command": "openOptions", "store": "options", "expect": "non_nil"},
                {"op": "invoke", "plugin": "CGMZ_MenuCommandWindow", "command": "refresh", "store": "commandWindow", "expect": "non_nil"},
                {"op": "set", "key": "summary", "value": {"from": "concat", "parts": [
                    {"from": "arg", "index": 0, "default": "reload_boot"},
                    ":",
                    {"from": "param", "name": "defaultRoute"},
                    ":",
                    {"from": "local", "name": "menu", "default": null}
                ]}},
                {"op": "returnObject"}
            ]
        }
    ]
})"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyResult = [&](const Value& value, const std::string& expectedFirstArg) {
                REQUIRE(std::holds_alternative<Object>(value.v));
                const auto& object = std::get<Object>(value.v);

                REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedFirstArg);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("menu").v).at("columnCount").v) == 2);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("options").v).at("profile").v) == "options_core");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("options").v).at("toggleCount").v) == 4);
                REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) == "command_window");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("commandWindow").v).at("visibleRows").v) == 8);
        };

        Value beforeReloadArg;
        beforeReloadArg.v = std::string("before_reload");
        const Value beforeReload = pm.executeCommand("CuratedMenuReloadFixture", "run", {beforeReloadArg});
        verifyResult(beforeReload, "before_reload");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("VisuStella_MainMenuCore_MZ"));
        REQUIRE(pm.reloadPlugin("VisuStella_OptionsCore_MZ"));
        REQUIRE(pm.reloadPlugin("CGMZ_MenuCommandWindow"));
        REQUIRE(pm.reloadPlugin("CuratedMenuReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedMenuReloadFixture", "run"));
        REQUIRE(pm.hasCommand("VisuStella_MainMenuCore_MZ", "openMenu"));

        Value afterReloadArg;
        afterReloadArg.v = std::string("after_reload");
        const Value afterReload = pm.executeCommand("CuratedMenuReloadFixture", "run", {afterReloadArg});
        verifyResult(afterReload, "after_reload");

        const Value byNameReload =
                pm.executeCommandByName("CuratedMenuReloadFixture_run", {afterReloadArg});
        verifyResult(byNameReload, "after_reload");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
}

    TEST_CASE("Compat fixtures: dependent command execution recovers after core reload",
                "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));

        REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
        REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

        const Value beforeUnloadMainMenu =
            pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
        REQUIRE(std::holds_alternative<Object>(beforeUnloadMainMenu.v));
        REQUIRE(std::get<std::string>(std::get<Object>(beforeUnloadMainMenu.v).at("profile").v) ==
                "menu_core");

        REQUIRE(pm.unloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE_FALSE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
        REQUIRE_FALSE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

        const Value gatedMainMenu =
            pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
        REQUIRE(std::holds_alternative<std::monostate>(gatedMainMenu.v));
        REQUIRE(
            pm.getLastError() ==
            "Missing dependencies for VisuStella_MainMenuCore_MZ_openMenu: VisuStella_CoreEngine_MZ"
        );

        const Value gatedOptions =
            pm.executeCommand("VisuStella_OptionsCore_MZ", "openOptions", {});
        REQUIRE(std::holds_alternative<std::monostate>(gatedOptions.v));
        REQUIRE(
            pm.getLastError() ==
            "Missing dependencies for VisuStella_OptionsCore_MZ_openOptions: VisuStella_CoreEngine_MZ"
        );

        const auto diagnosticsWhileMissing = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnosticsWhileMissing.find("execute_command_dependency_missing") != std::string::npos);

        pm.clearFailureDiagnostics();
        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));

        REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
        REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

        const Value recoveredMainMenu =
            pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
        REQUIRE(std::holds_alternative<Object>(recoveredMainMenu.v));
        const auto& recoveredMainMenuObject = std::get<Object>(recoveredMainMenu.v);
        REQUIRE(std::get<std::string>(recoveredMainMenuObject.at("profile").v) == "menu_core");
        REQUIRE(std::get<int64_t>(recoveredMainMenuObject.at("columnCount").v) == 2);

        const Value recoveredOptions =
            pm.executeCommandByName("VisuStella_OptionsCore_MZ_openOptions", {});
        REQUIRE(std::holds_alternative<Object>(recoveredOptions.v));
        const auto& recoveredOptionsObject = std::get<Object>(recoveredOptions.v);
        REQUIRE(std::get<std::string>(recoveredOptionsObject.at("profile").v) == "options_core");
        REQUIRE(std::get<int64_t>(recoveredOptionsObject.at("toggleCount").v) == 4);

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();
    }

TEST_CASE(
    "Compat fixtures: deterministic cross-plugin invoke chain fuzz matrix executes without diagnostics",
    "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto& specs = fixtureSpecs();
    for (const auto& spec : specs) {
        INFO("Fixture: " << spec.pluginName);
        REQUIRE(pm.loadPlugin(fixturePath(spec.pluginName).string()));
    }

    const auto fuzzFixture = uniqueTempFixturePath("urpg_dsl_invoke_fuzz_fixture");

    nlohmann::json fixture;
    fixture["name"] = "DslInvokeFuzzFixture";
    fixture["commands"] = nlohmann::json::array();

    constexpr size_t kCaseCount = 32;
    for (size_t caseId = 0; caseId < kCaseCount; ++caseId) {
        const auto& first = specs[(caseId * 7 + 3) % specs.size()];
        const auto& second = specs[(caseId * 5 + 1) % specs.size()];
        const auto& third = specs[(caseId * 9 + 2) % specs.size()];

        nlohmann::json script = nlohmann::json::array();
        script.push_back(
            {{"op", "set"}, {"key", "caseId"}, {"value", static_cast<int64_t>(caseId)}}
        );
        script.push_back(
            {
                {"op", "invoke"},
                {"plugin", first.pluginName},
                {"command", first.commandName},
                {"args",
                 nlohmann::json::array(
                     {nlohmann::json{
                         {"from", "arg"},
                         {"index", 0},
                         {"default", "first_default"},
                     }}
                 )},
                {"store", "first"},
                {"expect", "non_nil"},
            }
        );
        script.push_back(
            {
                {"op", "invokeByName"},
                {"name", first.pluginName + "_" + first.commandName},
                {"args",
                 nlohmann::json::array(
                     {nlohmann::json{
                         {"from", "arg"},
                         {"index", 0},
                         {"default", "first_default"},
                     }}
                 )},
                {"store", "firstByName"},
                {"expect", "non_nil"},
            }
        );
        script.push_back(
            {
                {"op", "invoke"},
                {"plugin", second.pluginName},
                {"command", second.commandName},
                {"args",
                 nlohmann::json::array(
                     {nlohmann::json{
                         {"from", "arg"},
                         {"index", 1},
                         {"default", "second_default"},
                     }}
                 )},
                {"store", "second"},
                {"expect", "non_nil"},
            }
        );
        script.push_back(
            {
                {"op", "if"},
                {"condition",
                 nlohmann::json{
                     {"from", "equals"},
                     {"left",
                      nlohmann::json{
                          {"from", "arg"},
                          {"index", 2},
                          {"default", 0},
                      }},
                     {"right", 1},
                 }},
                {"then",
                 nlohmann::json::array(
                     {nlohmann::json{
                         {"op", "invoke"},
                         {"plugin", third.pluginName},
                         {"command", third.commandName},
                         {"args",
                          nlohmann::json::array(
                              {nlohmann::json{
                                  {"from", "arg"},
                                  {"index", 2},
                                  {"default", 1},
                              }}
                          )},
                         {"store", "third"},
                         {"expect", "non_nil"},
                     }}
                 )},
                {"else",
                 nlohmann::json::array(
                     {nlohmann::json{
                         {"op", "invokeByName"},
                         {"name", third.pluginName + "_" + third.commandName},
                         {"args",
                          nlohmann::json::array(
                              {nlohmann::json{
                                  {"from", "arg"},
                                  {"index", 1},
                                  {"default", "third_default"},
                              }}
                          )},
                         {"store", "third"},
                         {"expect", "non_nil"},
                     }}
                 )},
            }
        );
        script.push_back({{"op", "returnObject"}});

        fixture["commands"].push_back(
            {
                {"name", "chainCase" + std::to_string(caseId)},
                {"script", std::move(script)},
            }
        );
    }

    const std::string fixtureText = fixture.dump(2);
    writeTextFile(fuzzFixture, fixtureText);

    REQUIRE(pm.loadPlugin(fuzzFixture.string()));

    for (size_t caseId = 0; caseId < kCaseCount; ++caseId) {
        const auto& first = specs[(caseId * 7 + 3) % specs.size()];
        const auto& second = specs[(caseId * 5 + 1) % specs.size()];
        const auto& third = specs[(caseId * 9 + 2) % specs.size()];

        Value arg0;
        arg0.v = std::string("seed_") + std::to_string(caseId);
        Value arg1;
        arg1.v = std::string("route_") + std::to_string(caseId);
        const Value arg2 = Value::Int((caseId % 2 == 0) ? 1 : 0);

        const Value result = pm.executeCommand(
            "DslInvokeFuzzFixture",
            "chainCase" + std::to_string(caseId),
            {arg0, arg1, arg2}
        );
        REQUIRE(std::holds_alternative<Object>(result.v));
        const auto& object = std::get<Object>(result.v);

        REQUIRE(std::holds_alternative<int64_t>(object.at("caseId").v));
        REQUIRE(std::get<int64_t>(object.at("caseId").v) == static_cast<int64_t>(caseId));

        const auto assertCommandObject =
            [&object](const std::string& key, const FixtureSpec& spec) {
                REQUIRE(std::holds_alternative<Object>(object.at(key).v));
                const auto& nested = std::get<Object>(object.at(key).v);
                REQUIRE(std::holds_alternative<std::string>(nested.at("plugin").v));
                REQUIRE(std::get<std::string>(nested.at("plugin").v) == spec.pluginName);
                REQUIRE(std::holds_alternative<std::string>(nested.at("command").v));
                REQUIRE(std::get<std::string>(nested.at("command").v) == spec.commandName);
            };

        assertCommandObject("first", first);
        assertCommandObject("firstByName", first);
        assertCommandObject("second", second);
        assertCommandObject("third", third);
    }

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(fuzzFixture, ec);
}
