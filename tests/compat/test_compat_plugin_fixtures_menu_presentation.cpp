#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"
#include "runtimes/compat_js/window_compat.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace {

using urpg::Object;
using urpg::Value;
using urpg::compat::DataManager;
using urpg::compat::PluginManager;
using urpg::compat::Window_Base;

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
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
    return std::filesystem::temp_directory_path() / (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

} // namespace

TEST_CASE("Compat fixtures: curated menu-stack scenarios preserve plugin-specific behavior", "[compat][fixtures]") {
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
    writeTextFile(scenarioFixture,
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
})");

    REQUIRE(pm.loadPlugin(scenarioFixture.string()));

    const Value defaultResult = pm.executeCommand("CuratedMenuScenarioFixture", "run", {});
    REQUIRE(std::holds_alternative<Object>(defaultResult.v));
    const auto& defaultObject = std::get<Object>(defaultResult.v);
    REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("boot").v).at("plugin").v) ==
            "VisuStella_CoreEngine_MZ");
    REQUIRE(std::get<std::string>(std::get<Object>(defaultObject.at("menu").v).at("profile").v) == "menu_core");
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
    const Value layoutResult = pm.executeCommand("CuratedMenuScenarioFixture", "run", {layoutSeed, layoutRoute});
    REQUIRE(std::holds_alternative<Object>(layoutResult.v));
    const auto& layoutObject = std::get<Object>(layoutResult.v);
    REQUIRE(std::get<std::string>(std::get<Object>(layoutObject.at("boot").v).at("firstArg").v) == "layout_boot");
    REQUIRE(std::get<std::string>(layoutObject.at("route").v) == "layout");
    REQUIRE(std::get<std::string>(std::get<Object>(layoutObject.at("routeResult").v).at("layout").v) == "horizontal");
    REQUIRE(std::get<int64_t>(layoutObject.at("routeToken").v) == 2);
    REQUIRE(std::get<std::string>(layoutObject.at("summary").v) == "layout:options:2");

    Value battleSeed;
    battleSeed.v = std::string("battle_boot");
    Value battleRoute;
    battleRoute.v = std::string("battle");
    const Value battleResult = pm.executeCommand("CuratedMenuScenarioFixture", "run", {battleSeed, battleRoute});
    REQUIRE(std::holds_alternative<Object>(battleResult.v));
    const auto& battleObject = std::get<Object>(battleResult.v);
    REQUIRE(std::get<std::string>(battleObject.at("route").v) == "battle");
    REQUIRE(std::get<std::string>(std::get<Object>(battleObject.at("routeResult").v).at("profile").v) == "battle_hud");
    REQUIRE(std::get<int64_t>(std::get<Object>(battleObject.at("routeResult").v).at("hudSlots").v) == 4);
    REQUIRE(std::get<bool>(battleObject.at("routeToken").v));
    REQUIRE(std::get<std::string>(battleObject.at("summary").v) == "battle:options:true");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(scenarioFixture, ec);
}

TEST_CASE("Compat fixtures: curated menu-presentation scenarios survive plugin reload", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));
    REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_menu_presentation_reload_fixture");
    writeTextFile(reloadFixture,
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
})");

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
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) ==
                "command_window");
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
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) ==
                "command_window");
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
    const Value afterReloadLayout = pm.executeCommandByName("CuratedMenuPresentationReloadFixture_run", {layoutBoot});
    verifyLayoutRoute(afterReloadLayout, "after_reload_menu_layout");

    Value hudBoot;
    hudBoot.v = std::string("after_reload_menu_hud");
    Value hudRoute;
    hudRoute.v = std::string("hud");
    const Value afterReloadHud = pm.executeCommand("CuratedMenuPresentationReloadFixture", "run", {hudBoot, hudRoute});
    verifyHudRoute(afterReloadHud, "after_reload_menu_hud");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated presentation scenarios survive plugin reload", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("AltMenuScreen_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_presentation_reload_fixture");
    writeTextFile(reloadFixture,
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
})");

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

    auto verifyMotionRoute = [&](const Value& value, const std::string& expectedBoot,
                                 const std::string& expectedMotionName) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == "motion");
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotionName);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) ==
                "character_motion");
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
    const Value beforeReload = pm.executeCommand("CuratedPresentationReloadFixture", "run",
                                                 {beforeReloadBoot, beforeReloadRoute, beforeReloadMotion});
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
    const Value afterReloadHud = pm.executeCommand("CuratedPresentationReloadFixture", "run", {hudBoot, hudRoute});
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

TEST_CASE("Compat fixtures: curated menu-stack scenarios survive plugin reload", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("CGMZ_MenuCommandWindow").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_menu_reload_fixture");
    writeTextFile(reloadFixture,
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
})");

    REQUIRE(pm.loadPlugin(reloadFixture.string()));

    auto verifyResult = [&](const Value& value, const std::string& expectedFirstArg) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);

        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedFirstArg);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("menu").v).at("profile").v) == "menu_core");
        REQUIRE(std::get<int64_t>(std::get<Object>(object.at("menu").v).at("columnCount").v) == 2);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("options").v).at("profile").v) == "options_core");
        REQUIRE(std::get<int64_t>(std::get<Object>(object.at("options").v).at("toggleCount").v) == 4);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("commandWindow").v).at("profile").v) ==
                "command_window");
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

    const Value byNameReload = pm.executeCommandByName("CuratedMenuReloadFixture_run", {afterReloadArg});
    verifyResult(byNameReload, "after_reload");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: dependent command execution recovers after core reload", "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_MainMenuCore_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_OptionsCore_MZ").string()));

    REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    const Value beforeUnloadMainMenu = pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
    REQUIRE(std::holds_alternative<Object>(beforeUnloadMainMenu.v));
    REQUIRE(std::get<std::string>(std::get<Object>(beforeUnloadMainMenu.v).at("profile").v) == "menu_core");

    REQUIRE(pm.unloadPlugin("VisuStella_CoreEngine_MZ"));
    REQUIRE_FALSE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE_FALSE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    const Value gatedMainMenu = pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
    REQUIRE(std::holds_alternative<std::monostate>(gatedMainMenu.v));
    REQUIRE(pm.getLastError() ==
            "Missing dependencies for VisuStella_MainMenuCore_MZ_openMenu: VisuStella_CoreEngine_MZ");

    const Value gatedOptions = pm.executeCommand("VisuStella_OptionsCore_MZ", "openOptions", {});
    REQUIRE(std::holds_alternative<std::monostate>(gatedOptions.v));
    REQUIRE(pm.getLastError() ==
            "Missing dependencies for VisuStella_OptionsCore_MZ_openOptions: VisuStella_CoreEngine_MZ");

    const auto diagnosticsWhileMissing = pm.exportFailureDiagnosticsJsonl();
    REQUIRE(diagnosticsWhileMissing.find("execute_command_dependency_missing") != std::string::npos);

    pm.clearFailureDiagnostics();
    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));

    REQUIRE(pm.checkDependencies("VisuStella_MainMenuCore_MZ"));
    REQUIRE(pm.checkDependencies("VisuStella_OptionsCore_MZ"));

    const Value recoveredMainMenu = pm.executeCommand("VisuStella_MainMenuCore_MZ", "openMenu", {});
    REQUIRE(std::holds_alternative<Object>(recoveredMainMenu.v));
    const auto& recoveredMainMenuObject = std::get<Object>(recoveredMainMenu.v);
    REQUIRE(std::get<std::string>(recoveredMainMenuObject.at("profile").v) == "menu_core");
    REQUIRE(std::get<int64_t>(recoveredMainMenuObject.at("columnCount").v) == 2);

    const Value recoveredOptions = pm.executeCommandByName("VisuStella_OptionsCore_MZ_openOptions", {});
    REQUIRE(std::holds_alternative<Object>(recoveredOptions.v));
    const auto& recoveredOptionsObject = std::get<Object>(recoveredOptions.v);
    REQUIRE(std::get<std::string>(recoveredOptionsObject.at("profile").v) == "options_core");
    REQUIRE(std::get<int64_t>(recoveredOptionsObject.at("toggleCount").v) == 4);

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();
}
