// PluginManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Plugin Command Registry + Execution

#include "runtimes/compat_js/plugin_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <memory>
#include <stdexcept>

using namespace urpg::compat;

namespace {

std::filesystem::path fixturePath(const std::string& pluginName) {
    std::vector<std::filesystem::path> candidateRoots;
#ifdef URPG_SOURCE_DIR
    candidateRoots.push_back(
        std::filesystem::path(URPG_SOURCE_DIR) / "tests" / "compat" / "fixtures" / "plugins"
    );
#endif
    candidateRoots.push_back(
        std::filesystem::current_path() / "tests" / "compat" / "fixtures" / "plugins"
    );
    candidateRoots.push_back(
        std::filesystem::current_path().parent_path() / "tests" / "compat" / "fixtures" / "plugins"
    );
    candidateRoots.push_back(
        std::filesystem::path("tests") / "compat" / "fixtures" / "plugins"
    );

    for (const auto& root : candidateRoots) {
        const auto candidate = root / (pluginName + ".json");
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return candidateRoots.front() / (pluginName + ".json");
}

} // namespace

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

    SECTION("Reload fixture plugin uses tracked source path") {
        const auto fixture = fixturePath("VisuStella_CoreEngine_MZ");
        REQUIRE(std::filesystem::exists(fixture));

        REQUIRE(pm.loadPlugin(fixture.string()));
        REQUIRE(pm.hasCommand("VisuStella_CoreEngine_MZ", "boot"));

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.hasCommand("VisuStella_CoreEngine_MZ", "boot"));

        urpg::Value arg;
        arg.v = std::string("after_reload");
        const urpg::Value result =
            pm.executeCommand("VisuStella_CoreEngine_MZ", "boot", {arg});
        REQUIRE(std::holds_alternative<urpg::Object>(result.v));

        pm.unloadPlugin("VisuStella_CoreEngine_MZ");
    }
}

TEST_CASE("PluginManager: Command registration", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Register command") {
        pm.registerPlugin({"CommandTestPlugin", "1.0", "", ""});
        
        bool registered = pm.registerCommand(
            "CommandTestPlugin",
            "testCommand",
            [](const std::vector<urpg::Value>& args) -> urpg::Value {
                return urpg::Value();
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
            [](const std::vector<urpg::Value>&) -> urpg::Value {
                return urpg::Value();
            }
        );
        
        REQUIRE(pm.unregisterCommand("UnregCommandPlugin", "toUnregister"));
        REQUIRE_FALSE(pm.hasCommand("UnregCommandPlugin", "toUnregister"));
        
        pm.unregisterPlugin("UnregCommandPlugin");
    }
    
    SECTION("Unregister all commands for plugin") {
        pm.registerPlugin({"MultiCommandPlugin", "1.0", "", ""});
        
        pm.registerCommand("MultiCommandPlugin", "cmd1", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd2", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("MultiCommandPlugin", "cmd3", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        
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
            [&wasExecuted](const std::vector<urpg::Value>& args) -> urpg::Value {
                wasExecuted = true;
                return urpg::Value();
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
            [&wasExecuted](const std::vector<urpg::Value>&) -> urpg::Value {
                wasExecuted = true;
                return urpg::Value();
            }
        );
        
        pm.executeCommandByName("FullNamePlugin_myCommand", {});
        REQUIRE(wasExecuted);
        
        pm.unregisterPlugin("FullNamePlugin");
    }

    SECTION("Execute command by full name supports plugin names with underscores") {
        const auto fixture = fixturePath("VisuStella_CoreEngine_MZ");
        REQUIRE(std::filesystem::exists(fixture));
        REQUIRE(pm.loadPlugin(fixture.string()));

        const urpg::Value result = pm.executeCommandByName(
            "VisuStella_CoreEngine_MZ_countArgsViaJs",
            {urpg::Value::Int(1), urpg::Value::Int(2)}
        );
        REQUIRE(std::holds_alternative<int64_t>(result.v));
        REQUIRE(std::get<int64_t>(result.v) == 2);

        pm.unloadPlugin("VisuStella_CoreEngine_MZ");
    }
    
    SECTION("Execute non-existent command returns empty") {
        urpg::Value result = pm.executeCommand("NonExistent", "command", {});
        REQUIRE(result.v.index() == 0); // null/empty
    }

    SECTION("Execute command asynchronously with FIFO callback order") {
        pm.registerPlugin({"AsyncPlugin", "1.0", "", ""});
        pm.registerCommand(
            "AsyncPlugin",
            "echoInt",
            [](const std::vector<urpg::Value>& args) -> urpg::Value {
                if (args.empty()) {
                    return urpg::Value::Int(-1);
                }
                return args.front();
            }
        );

        std::mutex callbackMutex;
        std::condition_variable callbackCv;
        std::vector<int64_t> callbackOrder;
        int32_t completed = 0;
        bool callbackTypeMismatch = false;

        for (int32_t i = 0; i < 3; ++i) {
            pm.executeCommandAsync(
                "AsyncPlugin",
                "echoInt",
                {urpg::Value::Int(i)},
                [&](const urpg::Value& result) {
                    std::lock_guard<std::mutex> lock(callbackMutex);
                    if (std::holds_alternative<int64_t>(result.v)) {
                        callbackOrder.push_back(std::get<int64_t>(result.v));
                    } else {
                        callbackTypeMismatch = true;
                    }
                    ++completed;
                    callbackCv.notify_one();
                }
            );
        }

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            const bool done = callbackCv.wait_for(
                lock,
                std::chrono::seconds(2),
                [&]() { return completed == 3; }
            );
            REQUIRE(done);
        }

        REQUIRE_FALSE(callbackTypeMismatch);
        REQUIRE(callbackOrder.size() == 3);
        REQUIRE(callbackOrder[0] == 0);
        REQUIRE(callbackOrder[1] == 1);
        REQUIRE(callbackOrder[2] == 2);

        pm.unregisterPlugin("AsyncPlugin");
    }
}

TEST_CASE("PluginManager: Parameter management", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();
    
    SECTION("Set and get parameter") {
        pm.registerPlugin({"ParamPlugin", "1.0", "", ""});
        
        urpg::Value val;
        val.v = std::string("test_value");
        
        pm.setParameter("ParamPlugin", "myParam", val);
        
        urpg::Value retrieved = pm.getParameter("ParamPlugin", "myParam");
        REQUIRE(retrieved.v.index() == val.v.index());
        
        pm.unregisterPlugin("ParamPlugin");
    }
    
    SECTION("Get parameter with default") {
        pm.registerPlugin({"DefaultParamPlugin", "1.0", "", ""});
        
        urpg::Value defaultVal;
        defaultVal.v = std::string("default");
        
        urpg::Value retrieved = pm.getParameter("DefaultParamPlugin", "nonexistent", defaultVal);
        REQUIRE(std::get<std::string>(retrieved.v) == "default");
        
        pm.unregisterPlugin("DefaultParamPlugin");
    }
    
    SECTION("Get all parameters for plugin") {
        pm.registerPlugin({"MultiParamPlugin", "1.0", "", ""});
        
        urpg::Value val1, val2;
        val1.v = std::string("value1");
        val2.v = std::string("value2");
        
        pm.setParameter("MultiParamPlugin", "param1", val1);
        pm.setParameter("MultiParamPlugin", "param2", val2);
        
        auto params = pm.getParameters("MultiParamPlugin");
        REQUIRE(params.size() == 2);
        
        pm.unregisterPlugin("MultiParamPlugin");
    }

    SECTION("Parse parameters from JSON object") {
        pm.registerPlugin({"JsonParamPlugin", "1.0", "", ""});

        REQUIRE(pm.parseParameters("JsonParamPlugin", R"({"enabled":true,"retries":3,"name":"fixture"})"));

        const auto params = pm.getParameters("JsonParamPlugin");
        REQUIRE(params.size() == 3);
        REQUIRE(std::get<bool>(params.at("enabled").v));
        REQUIRE(std::get<int64_t>(params.at("retries").v) == 3);
        REQUIRE(std::get<std::string>(params.at("name").v) == "fixture");

        pm.unregisterPlugin("JsonParamPlugin");
    }

    SECTION("Parse parameters rejects invalid JSON") {
        REQUIRE_FALSE(pm.parseParameters("BrokenJsonPlugin", R"({"enabled":true,)"));
        REQUIRE(pm.getLastError().find("Parameter JSON") != std::string::npos);
        pm.clearLastError();
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
            [&pm, &contextWasSet](const std::vector<urpg::Value>&) -> urpg::Value {
                contextWasSet = pm.isExecuting();
                return urpg::Value();
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
            [&pm, &currentPlugin](const std::vector<urpg::Value>&) -> urpg::Value {
                currentPlugin = pm.getCurrentPlugin();
                return urpg::Value();
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
        pm.clearFailureDiagnostics();
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
            [](const std::vector<urpg::Value>&) -> urpg::Value {
                throw std::runtime_error("Test error");
            }
        );
        
        pm.executeCommand("ErrorPlugin", "throwingCommand", {});
        REQUIRE(errorHandlerCalled);
        
        pm.setErrorHandler(nullptr);
        pm.unregisterPlugin("ErrorPlugin");
    }

    SECTION("Failure diagnostics JSONL exports and clears structured failures") {
        pm.clearFailureDiagnostics();
        pm.executeCommand("MissingPlugin", "missingCommand", {});

        const std::string jsonl = pm.exportFailureDiagnosticsJsonl();
        REQUIRE_FALSE(jsonl.empty());
        REQUIRE(jsonl.find("\"subsystem\":\"plugin_manager\"") != std::string::npos);
        REQUIRE(jsonl.find("\"operation\":\"execute_command\"") != std::string::npos);
        REQUIRE(jsonl.find("\"plugin\":\"MissingPlugin\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());
    }

    SECTION("Invalid parameter JSON is captured in failure diagnostics") {
        pm.clearFailureDiagnostics();
        REQUIRE_FALSE(pm.parseParameters("BrokenParamPlugin", R"({"enabled":true,)"));

        const std::string jsonl = pm.exportFailureDiagnosticsJsonl();
        REQUIRE_FALSE(jsonl.empty());
        REQUIRE(jsonl.find("\"operation\":\"parse_parameters_json\"") != std::string::npos);
        REQUIRE(jsonl.find("\"plugin\":\"BrokenParamPlugin\"") != std::string::npos);

        pm.clearFailureDiagnostics();
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
    
    SECTION("GetMethodStatus returns FULL for async execution") {
        CompatStatus status = pm.getMethodStatus("executeCommandAsync");
        REQUIRE(status == CompatStatus::FULL);
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
