// PluginManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Plugin Command Registry + Execution

#include "runtimes/compat_js/plugin_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

using namespace urpg::compat;

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
}

TEST_CASE("PluginManager: Command registration", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Register command") {
        pm.registerPlugin({"CommandTestPlugin", "1.0", "", ""});
        
        bool registered = pm.registerCommand(
            "CommandTestPlugin",
            "testCommand",
            [](const std::vector<urpg::engine::Value>& args) -> urpg::engine::Value {
                return urpg::engine::Value();
            },
            "A test command"
        );
        
        REQUIRE(registered);
        REQUIRE(pm.hasCommand("CommandTestPlugin", "testCommand"));
        
        pm.unregisterPlugin("CommandTestPlugin");
    }
    
    SECTION("Cannot register command with null handler") {
        pm.registerPlugin({"NullHandlerPlugin", "1.0", "", ""});
        
        bool registered = pm.registerCommand(
            "NullHandlerPlugin",
            "nullCommand",
            nullptr
        );
        
        REQUIRE_FALSE(registered);
        
        pm.unregisterPlugin("NullHandlerPlugin");
    }
    
    SECTION("Unregister command") {
        pm.registerPlugin({"UnregCommandPlugin", "1.0", "", ""});
        
        pm.registerCommand(
            "UnregCommandPlugin",
            "toUnregister",
            [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                return urpg::engine::Value();
            }
        );
        
        REQUIRE(pm.unregisterCommand("UnregCommandPlugin", "toUnregister"));
        REQUIRE_FALSE(pm.hasCommand("UnregCommandPlugin", "toUnregister"));
        
        pm.unregisterPlugin("UnregCommandPlugin");
    }
    
    SECTION("Unregister all commands for plugin") {
        pm.registerPlugin({"MultiCommandPlugin", "1.0", "", ""});
        
        pm.registerCommand("MultiCommandPlugin", "cmd1", [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd2", [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd3", [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value { return {}; });
        
        int32_t count = pm.unregisterAllCommands("MultiCommandPlugin");
        REQUIRE(count == 3);
        
        auto commands = pm.getPluginCommands("MultiCommandPlugin");
        REQUIRE(commands.empty());
        
        pm.unregisterPlugin("MultiCommandPlugin");
    }
}

TEST_CASE("PluginManager: Command execution", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Execute command") {
        pm.registerPlugin({"ExecPlugin", "1.0", "", ""});
        
        bool wasExecuted = false;
        pm.registerCommand(
            "ExecPlugin",
            "executeMe",
            [&wasExecuted](const std::vector<urpg::engine::Value>& args) -> urpg::engine::Value {
                wasExecuted = true;
                return urpg::engine::Value();
            }
        );
        
        pm.executeCommand("ExecPlugin", "executeMe", {});
        REQUIRE(wasExecuted);
        
        pm.unregisterPlugin("ExecPlugin");
    }
    
    SECTION("Execute command by full name") {
        pm.registerPlugin({"FullNamePlugin", "1.0", "", ""});
        
        bool wasExecuted = false;
        pm.registerCommand(
            "FullNamePlugin",
            "myCommand",
            [&wasExecuted](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                wasExecuted = true;
                return urpg::engine::Value();
            }
        );
        
        pm.executeCommandByName("FullNamePlugin_myCommand", {});
        REQUIRE(wasExecuted);
        
        pm.unregisterPlugin("FullNamePlugin");
    }
    
    SECTION("Execute non-existent command returns empty") {
        urpg::engine::Value result = pm.executeCommand("NonExistent", "command", {});
        REQUIRE(result.v.index() == 0); // null/empty
    }
}

TEST_CASE("PluginManager: Parameter management", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Set and get parameter") {
        pm.registerPlugin({"ParamPlugin", "1.0", "", ""});
        
        urpg::engine::Value val;
        val.v = std::string("test_value");
        
        pm.setParameter("ParamPlugin", "myParam", val);
        
        urpg::engine::Value retrieved = pm.getParameter("ParamPlugin", "myParam");
        REQUIRE(retrieved.v.index() == val.v.index());
        
        pm.unregisterPlugin("ParamPlugin");
    }
    
    SECTION("Get parameter with default") {
        pm.registerPlugin({"DefaultParamPlugin", "1.0", "", ""});
        
        urpg::engine::Value defaultVal;
        defaultVal.v = std::string("default");
        
        urpg::engine::Value retrieved = pm.getParameter("DefaultParamPlugin", "nonexistent", defaultVal);
        // Should return default for non-existent parameter
        
        pm.unregisterPlugin("DefaultParamPlugin");
    }
    
    SECTION("Get all parameters for plugin") {
        pm.registerPlugin({"MultiParamPlugin", "1.0", "", ""});
        
        urpg::engine::Value val1, val2;
        val1.v = std::string("value1");
        val2.v = std::string("value2");
        
        pm.setParameter("MultiParamPlugin", "param1", val1);
        pm.setParameter("MultiParamPlugin", "param2", val2);
        
        auto params = pm.getParameters("MultiParamPlugin");
        REQUIRE(params.size() == 2);
        
        pm.unregisterPlugin("MultiParamPlugin");
    }
}

TEST_CASE("PluginManager: Dependencies", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Check satisfied dependencies") {
        PluginInfo core;
        core.name = "CorePlugin";
        PluginInfo dependent;
        dependent.name = "DependentPlugin";
        dependent.dependencies = {"CorePlugin"};
        
        pm.registerPlugin(core);
        pm.registerPlugin(dependent);
        
        REQUIRE(pm.checkDependencies("DependentPlugin"));
        
        pm.unregisterPlugin("DependentPlugin");
        pm.unregisterPlugin("CorePlugin");
    }
    
    SECTION("Get missing dependencies") {
        PluginInfo info;
        info.name = "MissingDepPlugin";
        info.dependencies = {"NonExistentPlugin1", "NonExistentPlugin2"};
        
        pm.registerPlugin(info);
        
        auto missing = pm.getMissingDependencies("MissingDepPlugin");
        REQUIRE(missing.size() == 2);
        
        pm.unregisterPlugin("MissingDepPlugin");
    }
    
    SECTION("Get dependents") {
        PluginInfo core;
        core.name = "CorePlugin2";
        PluginInfo dep1;
        dep1.name = "Dependent1";
        dep1.dependencies = {"CorePlugin2"};
        PluginInfo dep2;
        dep2.name = "Dependent2";
        dep2.dependencies = {"CorePlugin2"};
        
        pm.registerPlugin(core);
        pm.registerPlugin(dep1);
        pm.registerPlugin(dep2);
        
        auto dependents = pm.getDependents("CorePlugin2");
        REQUIRE(dependents.size() == 2);
        
        pm.unregisterPlugin("Dependent1");
        pm.unregisterPlugin("Dependent2");
        pm.unregisterPlugin("CorePlugin2");
    }
}

TEST_CASE("PluginManager: Event handlers", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Register and trigger event handler") {
        bool eventTriggered = false;
        
        int32_t handlerId = pm.registerEventHandler(
            PluginManager::PluginEvent::ON_LOAD,
            [&eventTriggered](const std::string& pluginName, PluginManager::PluginEvent) {
                eventTriggered = true;
            }
        );
        
        REQUIRE(handlerId >= 0);
        
        PluginInfo info;
        info.name = "EventTestPlugin";
        pm.registerPlugin(info);
        
        REQUIRE(eventTriggered);
        
        pm.unregisterEventHandler(handlerId);
        pm.unregisterPlugin("EventTestPlugin");
    }
    
    SECTION("Unregister event handler") {
        int32_t handlerId = pm.registerEventHandler(
            PluginManager::PluginEvent::ON_LOAD,
            [](const std::string&, PluginManager::PluginEvent) {}
        );
        
        pm.unregisterEventHandler(handlerId);
        // Handler should no longer be called
    }
}

TEST_CASE("PluginManager: Execution context", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Not executing initially") {
        REQUIRE_FALSE(pm.isExecuting());
    }
    
    SECTION("Context is set during execution") {
        pm.registerPlugin({"ContextPlugin", "1.0", "", ""});
        
        bool contextWasSet = false;
        pm.registerCommand(
            "ContextPlugin",
            "checkContext",
            [&pm, &contextWasSet](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                contextWasSet = pm.isExecuting();
                return urpg::engine::Value();
            }
        );
        
        pm.executeCommand("ContextPlugin", "checkContext", {});
        REQUIRE(contextWasSet);
        
        pm.unregisterPlugin("ContextPlugin");
    }
    
    SECTION("Get current plugin during execution") {
        pm.registerPlugin({"CurrentPluginTest", "1.0", "", ""});
        
        std::string currentPlugin;
        pm.registerCommand(
            "CurrentPluginTest",
            "getCurrentPlugin",
            [&pm, &currentPlugin](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                currentPlugin = pm.getCurrentPlugin();
                return urpg::engine::Value();
            }
        );
        
        pm.executeCommand("CurrentPluginTest", "getCurrentPlugin", {});
        REQUIRE(currentPlugin == "CurrentPluginTest");
        
        pm.unregisterPlugin("CurrentPluginTest");
    }
}

TEST_CASE("PluginManager: Error handling", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Get last error after failure") {
        pm.unloadPlugin("NonExistentPlugin");
        REQUIRE_FALSE(pm.getLastError().empty());
        
        pm.clearLastError();
        REQUIRE(pm.getLastError().empty());
    }
    
    SECTION("Error handler is called on exception") {
        pm.registerPlugin({"ErrorPlugin", "1.0", "", ""});
        
        bool errorHandlerCalled = false;
        pm.setErrorHandler([&errorHandlerCalled](const std::string&, const std::string&, const std::string&) {
            errorHandlerCalled = true;
        });
        
        pm.registerCommand(
            "ErrorPlugin",
            "throwingCommand",
            [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                throw std::runtime_error("Test error");
            }
        );
        
        pm.executeCommand("ErrorPlugin", "throwingCommand", {});
        REQUIRE(errorHandlerCalled);
        
        pm.setErrorHandler(nullptr);
        pm.unregisterPlugin("ErrorPlugin");
    }
}

TEST_CASE("PluginManager: Method status registry", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("GetMethodStatus returns FULL for core methods") {
        CompatStatus status = pm.getMethodStatus("registerPlugin");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns FULL for command methods") {
        CompatStatus status = pm.getMethodStatus("registerCommand");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns PARTIAL for async execution") {
        CompatStatus status = pm.getMethodStatus("executeCommandAsync");
        REQUIRE(status == CompatStatus::PARTIAL);
    }
    
    SECTION("GetMethodStatus returns UNSUPPORTED for unknown methods") {
        CompatStatus status = pm.getMethodStatus("nonexistentMethod");
        REQUIRE(status == CompatStatus::UNSUPPORTED);
    }
}

TEST_CASE("PluginInfo: Structure defaults", "[plugin_manager]") {
    PluginInfo info;
    
    SECTION("Default values") {
        REQUIRE(info.name.empty());
        REQUIRE(info.version.empty());
        REQUIRE(info.author.empty());
        REQUIRE(info.description.empty());
        REQUIRE(info.dependencies.empty());
        REQUIRE(info.parameters.empty());
        REQUIRE(info.enabled);
        REQUIRE_FALSE(info.loaded);
    }
}

TEST_CASE("CommandInfo: Structure defaults", "[plugin_manager]") {
    CommandInfo info;
    
    SECTION("Default values") {
        REQUIRE(info.pluginName.empty());
        REQUIRE(info.commandName.empty());
        REQUIRE(info.description.empty());
        REQUIRE_FALSE(info.handler);
        REQUIRE(info.argNames.empty());
    }
}

TEST_CASE("PluginContext: Structure defaults", "[plugin_manager]") {
    PluginContext ctx;
    
    SECTION("Default values") {
        REQUIRE(ctx.pluginName.empty());
        REQUIRE(ctx.currentCommand.empty());
        REQUIRE(ctx.callDepth == 0);
        REQUIRE_FALSE(ctx.isYielding);
    }
}
