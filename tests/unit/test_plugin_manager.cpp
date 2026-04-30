// PluginManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Plugin registration and lifecycle behavior.

#include "tests/unit/plugin_manager_test_helpers.h"

TEST_CASE("PluginManager: Plugin registration", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Register plugin with info") {
        PluginInfo info;
        info.name = "TestPlugin";
        info.version = "1.0.0";
        info.author = "Test Author";
        info.description = "A test plugin";

        REQUIRE(pm.registerPlugin(info));
        REQUIRE(pm.isPluginLoaded("TestPlugin"));

        const PluginInfo* loaded = pm.getPluginInfo("TestPlugin");
        REQUIRE(loaded != nullptr);
        REQUIRE(loaded->name == "TestPlugin");
        REQUIRE(loaded->version == "1.0.0");

        pm.unregisterPlugin("TestPlugin");
    }

    SECTION("Cannot register plugin with empty name") {
        PluginInfo info;
        info.name = "";

        REQUIRE_FALSE(pm.registerPlugin(info));
    }

    SECTION("Cannot register duplicate plugin") {
        PluginInfo info;
        info.name = "DuplicatePlugin";

        REQUIRE(pm.registerPlugin(info));
        REQUIRE_FALSE(pm.registerPlugin(info));

        pm.unregisterPlugin("DuplicatePlugin");
    }
}

TEST_CASE("PluginManager: Plugin lifecycle", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Load plugin from path") {
        bool result = pm.loadPlugin("plugins/test_plugin.js");
        REQUIRE(result);

        pm.unloadPlugin("test_plugin");
    }

    SECTION("Load live JS plugin evaluates source and registers commands") {
        const auto tempDir = uniqueTempDirectoryPath("urpg_live_js_plugin");
        REQUIRE(std::filesystem::create_directories(tempDir));
        const auto pluginPath = tempDir / "LiveCommandPlugin.js";

        writeTextFile(pluginPath,
                      R"JS(/*:
 * @target MZ
 * @plugindesc Live plugin command fixture
 * @author URPG Test
 */
PluginManager.registerCommand("LiveCommandPlugin", "echo", function(args) {
  return {
    seen: args.message,
    enabled: PluginManager.parameters("LiveCommandPlugin").enabled
  };
});
)JS");

        urpg::Value enabled;
        enabled.v = true;
        pm.setParameter("LiveCommandPlugin", "enabled", enabled);

        REQUIRE(pm.loadPlugin(pluginPath.string()));
        REQUIRE(pm.isPluginLoaded("LiveCommandPlugin"));
        REQUIRE(pm.hasCommand("LiveCommandPlugin", "echo"));

        const PluginInfo* info = pm.getPluginInfo("LiveCommandPlugin");
        REQUIRE(info != nullptr);
        REQUIRE(info->description == "Live plugin command fixture");
        REQUIRE(info->author == "URPG Test");

        urpg::Object payload;
        urpg::Value message;
        message.v = std::string("from_live_js");
        payload["message"] = message;

        const urpg::Value result = pm.executeCommand("LiveCommandPlugin", "echo", {urpg::Value::Obj(payload)});
        REQUIRE(std::holds_alternative<urpg::Object>(result.v));
        const auto& object = std::get<urpg::Object>(result.v);
        REQUIRE(std::get<std::string>(object.at("seen").v) == "from_live_js");
        REQUIRE(std::get<bool>(object.at("enabled").v));

        REQUIRE(pm.reloadPlugin("LiveCommandPlugin"));
        REQUIRE(pm.hasCommand("LiveCommandPlugin", "echo"));

        pm.unloadPlugin("LiveCommandPlugin");
        std::error_code ec;
        std::filesystem::remove_all(tempDir, ec);
    }

    SECTION("Live JS plugin load failure rolls back and reports diagnostics") {
        const auto tempDir = uniqueTempDirectoryPath("urpg_bad_live_js_plugin");
        REQUIRE(std::filesystem::create_directories(tempDir));
        const auto pluginPath = tempDir / "BadLiveCommandPlugin.js";

        writeTextFile(pluginPath,
                      R"JS(PluginManager.registerCommand("BadLiveCommandPlugin", "broken", function(args) {
)JS");

        REQUIRE_FALSE(pm.loadPlugin(pluginPath.string()));
        REQUIRE_FALSE(pm.isPluginLoaded("BadLiveCommandPlugin"));
        REQUIRE_FALSE(pm.hasCommand("BadLiveCommandPlugin", "broken"));
        const std::string lastError = pm.getLastError();
        const bool hasSyntaxError = lastError.find("unexpected") != std::string::npos ||
                                    lastError.find("expecting") != std::string::npos ||
                                    lastError.find("syntax") != std::string::npos;
        REQUIRE(hasSyntaxError);

        const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
        REQUIRE_FALSE(diagnostics.empty());
        REQUIRE(diagnostics.back().value("operation", "") == "load_plugin_js_eval");

        std::error_code ec;
        std::filesystem::remove_all(tempDir, ec);
    }

    SECTION("Unload plugin") {
        PluginInfo info;
        info.name = "UnloadTestPlugin";
        pm.registerPlugin(info);

        REQUIRE(pm.unloadPlugin("UnloadTestPlugin"));
        REQUIRE_FALSE(pm.isPluginLoaded("UnloadTestPlugin"));
    }

    SECTION("Unload non-existent plugin fails") {
        REQUIRE_FALSE(pm.unloadPlugin("NonExistentPlugin"));
    }

    SECTION("Get loaded plugins list") {
        PluginInfo info1;
        info1.name = "Plugin1";
        PluginInfo info2;
        info2.name = "Plugin2";

        pm.registerPlugin(info1);
        pm.registerPlugin(info2);

        auto loaded = pm.getLoadedPlugins();
        REQUIRE(loaded.size() >= 2);

        pm.unregisterPlugin("Plugin1");
        pm.unregisterPlugin("Plugin2");
    }

    SECTION("Load plugins from directory uses deterministic lexical order") {
        const auto tempDir = uniqueTempDirectoryPath("urpg_plugin_order");
        REQUIRE(std::filesystem::create_directories(tempDir));

        const auto pluginCPath = tempDir / "c_fixture.json";
        const auto pluginAPath = tempDir / "a_fixture.json";
        const auto pluginBPath = tempDir / "b_fixture.json";

        writeTextFile(pluginCPath, R"({"name":"OrderPluginC","commands":[{"name":"probe","mode":"arg_count"}]})");
        writeTextFile(pluginAPath, R"({"name":"OrderPluginA","commands":[{"name":"probe","mode":"arg_count"}]})");
        writeTextFile(pluginBPath, R"({"name":"OrderPluginB","commands":[{"name":"probe","mode":"arg_count"}]})");

        std::vector<std::string> loadOrder;
        const int32_t handlerId =
            pm.registerEventHandler(PluginManager::PluginEvent::ON_LOAD,
                                    [&loadOrder](const std::string& pluginName, PluginManager::PluginEvent) {
                                        loadOrder.push_back(pluginName);
                                    });

        const auto loadedCount = pm.loadPluginsFromDirectory(tempDir.string());
        REQUIRE(loadedCount == 3);
        REQUIRE(loadOrder.size() == 3);
        REQUIRE(loadOrder[0] == "OrderPluginA");
        REQUIRE(loadOrder[1] == "OrderPluginB");
        REQUIRE(loadOrder[2] == "OrderPluginC");

        const auto loadedPlugins = pm.getLoadedPlugins();
        REQUIRE(loadedPlugins.size() >= 3);
        const auto aIt = std::find(loadedPlugins.begin(), loadedPlugins.end(), "OrderPluginA");
        const auto bIt = std::find(loadedPlugins.begin(), loadedPlugins.end(), "OrderPluginB");
        const auto cIt = std::find(loadedPlugins.begin(), loadedPlugins.end(), "OrderPluginC");
        REQUIRE(aIt != loadedPlugins.end());
        REQUIRE(bIt != loadedPlugins.end());
        REQUIRE(cIt != loadedPlugins.end());
        REQUIRE(aIt < bIt);
        REQUIRE(bIt < cIt);

        pm.unregisterEventHandler(handlerId);
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove_all(tempDir, ec);
    }

    SECTION("Reload fixture plugin uses tracked source path") {
        const auto fixture = fixturePath("VisuStella_CoreEngine_MZ");
        REQUIRE(std::filesystem::exists(fixture));

        REQUIRE(pm.loadPlugin(fixture.string()));
        REQUIRE(pm.hasCommand("VisuStella_CoreEngine_MZ", "boot"));

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.hasCommand("VisuStella_CoreEngine_MZ", "boot"));

        urpg::Value arg;
        arg.v = std::string("after_reload");
        const urpg::Value result = pm.executeCommand("VisuStella_CoreEngine_MZ", "boot", {arg});
        REQUIRE(std::holds_alternative<urpg::Object>(result.v));

        pm.unloadPlugin("VisuStella_CoreEngine_MZ");
    }
}

TEST_CASE("PluginManager: Command registration", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Register command") {
        pm.registerPlugin(makePluginInfo("CommandTestPlugin"));

        bool registered = pm.registerCommand(
            "CommandTestPlugin", "testCommand",
            [](const std::vector<urpg::Value>&) -> urpg::Value { return urpg::Value(); }, "A test command");

        REQUIRE(registered);
        REQUIRE(pm.hasCommand("CommandTestPlugin", "testCommand"));

        pm.unregisterPlugin("CommandTestPlugin");
    }

    SECTION("Cannot register command with null handler") {
        pm.registerPlugin(makePluginInfo("NullHandlerPlugin"));

        bool registered = pm.registerCommand("NullHandlerPlugin", "nullCommand", nullptr);

        REQUIRE_FALSE(registered);

        pm.unregisterPlugin("NullHandlerPlugin");
    }

    SECTION("Unregister command") {
        pm.registerPlugin(makePluginInfo("UnregCommandPlugin"));

        pm.registerCommand("UnregCommandPlugin", "toUnregister",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return urpg::Value(); });

        REQUIRE(pm.unregisterCommand("UnregCommandPlugin", "toUnregister"));
        REQUIRE_FALSE(pm.hasCommand("UnregCommandPlugin", "toUnregister"));

        pm.unregisterPlugin("UnregCommandPlugin");
    }

    SECTION("Unregister all commands for plugin") {
        pm.registerPlugin(makePluginInfo("MultiCommandPlugin"));

        pm.registerCommand("MultiCommandPlugin", "cmd1",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd2",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd3",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });

        int32_t count = pm.unregisterAllCommands("MultiCommandPlugin");
        REQUIRE(count == 3);

        auto commands = pm.getPluginCommands("MultiCommandPlugin");
        REQUIRE(commands.empty());

        pm.unregisterPlugin("MultiCommandPlugin");
    }

    SECTION("Get plugin commands returns deterministic lexical order") {
        pm.registerPlugin(makePluginInfo("SortedCommandPlugin"));

        pm.registerCommand("SortedCommandPlugin", "zeta",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("SortedCommandPlugin", "alpha",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("SortedCommandPlugin", "middle",
                           [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });

        const auto commands = pm.getPluginCommands("SortedCommandPlugin");
        REQUIRE(commands.size() == 3);
        REQUIRE(commands[0] == "alpha");
        REQUIRE(commands[1] == "middle");
        REQUIRE(commands[2] == "zeta");

        pm.unregisterPlugin("SortedCommandPlugin");
    }
}
