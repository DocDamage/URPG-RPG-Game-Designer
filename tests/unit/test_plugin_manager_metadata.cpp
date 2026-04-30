// PluginManager metadata, diagnostics, and structure tests.

#include "tests/unit/plugin_manager_test_helpers.h"

TEST_CASE("PluginManager: Parameter management", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Set and get parameter") {
        pm.registerPlugin(makePluginInfo("ParamPlugin"));

        urpg::Value val;
        val.v = std::string("test_value");

        pm.setParameter("ParamPlugin", "myParam", val);

        urpg::Value retrieved = pm.getParameter("ParamPlugin", "myParam");
        REQUIRE(retrieved.v.index() == val.v.index());

        pm.unregisterPlugin("ParamPlugin");
    }

    SECTION("Get parameter with default") {
        pm.registerPlugin(makePluginInfo("DefaultParamPlugin"));

        urpg::Value defaultVal;
        defaultVal.v = std::string("default");

        urpg::Value retrieved = pm.getParameter("DefaultParamPlugin", "nonexistent", defaultVal);
        REQUIRE(std::get<std::string>(retrieved.v) == "default");

        pm.unregisterPlugin("DefaultParamPlugin");
    }

    SECTION("Get all parameters for plugin") {
        pm.registerPlugin(makePluginInfo("MultiParamPlugin"));

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
        pm.registerPlugin(makePluginInfo("JsonParamPlugin"));

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

    SECTION("Dependency metadata is normalized before dependency checks") {
        PluginInfo info;
        info.name = "NoisyDepPlugin";
        info.dependencies = {"", "  ", "MissingDepB", "MissingDepA", "MissingDepA"};

        pm.registerPlugin(info);

        const auto* stored = pm.getPluginInfo("NoisyDepPlugin");
        REQUIRE(stored != nullptr);
        REQUIRE(stored->dependencies == std::vector<std::string>{"MissingDepA", "MissingDepB"});

        const auto missing = pm.getMissingDependencies("NoisyDepPlugin");
        REQUIRE(missing == std::vector<std::string>{"MissingDepA", "MissingDepB"});

        pm.unregisterPlugin("NoisyDepPlugin");
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

    SECTION("Get dependents returns deterministic lexical order") {
        PluginInfo core;
        core.name = "CorePluginSorted";
        PluginInfo depZ;
        depZ.name = "DependentZ";
        depZ.dependencies = {"CorePluginSorted"};
        PluginInfo depA;
        depA.name = "DependentA";
        depA.dependencies = {"CorePluginSorted"};
        PluginInfo depM;
        depM.name = "DependentM";
        depM.dependencies = {"CorePluginSorted"};

        pm.registerPlugin(core);
        pm.registerPlugin(depZ);
        pm.registerPlugin(depA);
        pm.registerPlugin(depM);

        const auto dependents = pm.getDependents("CorePluginSorted");
        REQUIRE(dependents.size() == 3);
        REQUIRE(dependents[0] == "DependentA");
        REQUIRE(dependents[1] == "DependentM");
        REQUIRE(dependents[2] == "DependentZ");

        pm.unregisterPlugin("DependentZ");
        pm.unregisterPlugin("DependentA");
        pm.unregisterPlugin("DependentM");
        pm.unregisterPlugin("CorePluginSorted");
    }
}

TEST_CASE("PluginManager: Event handlers", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Register and trigger event handler") {
        bool eventTriggered = false;

        int32_t handlerId = pm.registerEventHandler(
            PluginManager::PluginEvent::ON_LOAD,
            [&eventTriggered](const std::string&, PluginManager::PluginEvent) { eventTriggered = true; });

        REQUIRE(handlerId >= 0);

        PluginInfo info;
        info.name = "EventTestPlugin";
        pm.registerPlugin(info);

        REQUIRE(eventTriggered);

        pm.unregisterEventHandler(handlerId);
        pm.unregisterPlugin("EventTestPlugin");
    }

    SECTION("Unregister event handler") {
        int32_t handlerId = pm.registerEventHandler(PluginManager::PluginEvent::ON_LOAD,
                                                    [](const std::string&, PluginManager::PluginEvent) {});

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
        pm.registerPlugin(makePluginInfo("ContextPlugin"));

        bool contextWasSet = false;
        pm.registerCommand("ContextPlugin", "checkContext",
                           [&pm, &contextWasSet](const std::vector<urpg::Value>&) -> urpg::Value {
                               contextWasSet = pm.isExecuting();
                               return urpg::Value();
                           });

        pm.executeCommand("ContextPlugin", "checkContext", {});
        REQUIRE(contextWasSet);

        pm.unregisterPlugin("ContextPlugin");
    }

    SECTION("Get current plugin during execution") {
        pm.registerPlugin(makePluginInfo("CurrentPluginTest"));

        std::string currentPlugin;
        pm.registerCommand("CurrentPluginTest", "getCurrentPlugin",
                           [&pm, &currentPlugin](const std::vector<urpg::Value>&) -> urpg::Value {
                               currentPlugin = pm.getCurrentPlugin();
                               return urpg::Value();
                           });

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
        pm.registerPlugin(makePluginInfo("ErrorPlugin"));

        bool errorHandlerCalled = false;
        pm.setErrorHandler([&errorHandlerCalled](const std::string&, const std::string&, const std::string&) {
            errorHandlerCalled = true;
        });

        pm.registerCommand("ErrorPlugin", "throwingCommand", [](const std::vector<urpg::Value>&) -> urpg::Value {
            throw std::runtime_error("Test error");
        });

        pm.executeCommand("ErrorPlugin", "throwingCommand", {});
        REQUIRE(errorHandlerCalled);

        pm.setErrorHandler(nullptr);
        pm.unregisterPlugin("ErrorPlugin");
    }

    SECTION("Unknown command exceptions are tagged as crash prevented diagnostics") {
        pm.clearFailureDiagnostics();
        REQUIRE(pm.registerPlugin(makePluginInfo("CrashPreventedPlugin")));
        REQUIRE(pm.registerCommand("CrashPreventedPlugin", "throwUnknown",
                                   [](const std::vector<urpg::Value>&) -> urpg::Value { throw 7; }));

        const urpg::Value result = pm.executeCommand("CrashPreventedPlugin", "throwUnknown", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "Unknown command execution error");

        const auto rows = parseJsonl(pm.exportFailureDiagnosticsJsonl());
        REQUIRE_FALSE(rows.empty());
        const auto rowIt = std::find_if(rows.begin(), rows.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command" &&
                   row.value("plugin", "") == "CrashPreventedPlugin" && row.value("command", "") == "throwUnknown";
        });
        REQUIRE(rowIt != rows.end());
        REQUIRE(rowIt->value("severity", "") == "CRASH_PREVENTED");
        REQUIRE(rowIt->value("message", "") == "Unknown command execution error");

        pm.clearFailureDiagnostics();
        pm.unregisterPlugin("CrashPreventedPlugin");
    }

    SECTION("Failure diagnostics JSONL exports and clears structured failures") {
        pm.clearFailureDiagnostics();
        pm.executeCommand("MissingPlugin", "missingCommand", {});

        const std::string jsonl = pm.exportFailureDiagnosticsJsonl();
        REQUIRE_FALSE(jsonl.empty());
        REQUIRE(jsonl.find("\"subsystem\":\"plugin_manager\"") != std::string::npos);
        REQUIRE(jsonl.find("\"operation\":\"execute_command\"") != std::string::npos);
        REQUIRE(jsonl.find("\"plugin\":\"MissingPlugin\"") != std::string::npos);
        REQUIRE(jsonl.find("\"severity\":\"WARN\"") != std::string::npos);

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
        REQUIRE(jsonl.find("\"severity\":\"HARD_FAIL\"") != std::string::npos);

        pm.clearFailureDiagnostics();
    }

    SECTION("Failure diagnostics JSONL retains last bounded window with monotonic sequence IDs") {
        pm.clearFailureDiagnostics();

        constexpr size_t kTotalFailures = 2051;
        constexpr size_t kRetainedFailures = 2048;

        for (size_t i = 0; i < kTotalFailures; ++i) {
            pm.executeCommand("BoundedDiagPlugin", "missing_" + std::to_string(i), {});
        }

        const auto rows = parseJsonl(pm.exportFailureDiagnosticsJsonl());
        REQUIRE(rows.size() == kRetainedFailures);

        const uint64_t firstSeq = rows.front().value("seq", static_cast<uint64_t>(0));
        const uint64_t lastSeq = rows.back().value("seq", static_cast<uint64_t>(0));
        REQUIRE(lastSeq > firstSeq);
        REQUIRE(lastSeq - firstSeq + 1 == static_cast<uint64_t>(kRetainedFailures));

        for (size_t i = 1; i < rows.size(); ++i) {
            const uint64_t prevSeq = rows[i - 1].value("seq", static_cast<uint64_t>(0));
            const uint64_t seq = rows[i].value("seq", static_cast<uint64_t>(0));
            REQUIRE(seq == prevSeq + 1);
        }

        REQUIRE(rows.front().value("command", "") == "missing_3");
        REQUIRE(rows.back().value("command", "") == "missing_2050");
        REQUIRE(rows.front().value("operation", "") == "execute_command");
        REQUIRE(rows.back().value("operation", "") == "execute_command");
        REQUIRE(rows.front().value("severity", "") == "WARN");
        REQUIRE(rows.back().value("severity", "") == "WARN");

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

    SECTION("GetMethodStatus returns FULL for lifecycle and diagnostics methods") {
        REQUIRE(pm.getMethodStatus("unloadPlugin") == CompatStatus::FULL);
        REQUIRE(pm.getMethodStatus("getLastError") == CompatStatus::FULL);
        REQUIRE(pm.getMethodStatus("exportFailureDiagnosticsJsonl") == CompatStatus::FULL);
    }

    SECTION("GetMethodStatus returns FULL for async execution") {
        CompatStatus status = pm.getMethodStatus("executeCommandAsync");
        REQUIRE(status == CompatStatus::FULL);
        REQUIRE(pm.getMethodDeviation("executeCommandAsync").empty());
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
