#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
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

    pm.unloadAllPlugins();
}
