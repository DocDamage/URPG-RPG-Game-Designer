#include "runtimes/compat_js/plugin_manager.h"

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
using urpg::compat::PluginManager;

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
