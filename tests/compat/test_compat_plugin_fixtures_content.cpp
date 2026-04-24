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

} // namespace

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
            REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 1);
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


