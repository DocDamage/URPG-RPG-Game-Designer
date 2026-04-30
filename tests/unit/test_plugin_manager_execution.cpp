// PluginManager command execution tests.

#include "tests/unit/plugin_manager_test_helpers.h"

TEST_CASE("PluginManager: Command execution", "[plugin_manager]") {
    PluginManager& pm = PluginManager::instance();

    SECTION("Execute command") {
        pm.registerPlugin(makePluginInfo("ExecPlugin"));

        bool wasExecuted = false;
        pm.registerCommand("ExecPlugin", "executeMe", [&wasExecuted](const std::vector<urpg::Value>&) -> urpg::Value {
            wasExecuted = true;
            return urpg::Value();
        });

        pm.executeCommand("ExecPlugin", "executeMe", {});
        REQUIRE(wasExecuted);

        pm.unregisterPlugin("ExecPlugin");
    }

    SECTION("Execute command by full name") {
        pm.registerPlugin(makePluginInfo("FullNamePlugin"));

        bool wasExecuted = false;
        pm.registerCommand("FullNamePlugin", "myCommand",
                           [&wasExecuted](const std::vector<urpg::Value>&) -> urpg::Value {
                               wasExecuted = true;
                               return urpg::Value();
                           });

        pm.executeCommandByName("FullNamePlugin_myCommand", {});
        REQUIRE(wasExecuted);

        pm.unregisterPlugin("FullNamePlugin");
    }

    SECTION("Execute command by full name supports plugin names with underscores") {
        const auto fixture = fixturePath("VisuStella_CoreEngine_MZ");
        REQUIRE(std::filesystem::exists(fixture));
        REQUIRE(pm.loadPlugin(fixture.string()));

        const urpg::Value result = pm.executeCommandByName("VisuStella_CoreEngine_MZ_countArgsViaJs",
                                                           {urpg::Value::Int(1), urpg::Value::Int(2)});
        REQUIRE(std::holds_alternative<int64_t>(result.v));
        REQUIRE(std::get<int64_t>(result.v) == 2);

        pm.unloadPlugin("VisuStella_CoreEngine_MZ");
    }

    SECTION("Execute command by full name supports commands with underscores") {
        REQUIRE(pm.registerPlugin(makePluginInfo("FullNameCommandPlugin")));
        REQUIRE(pm.registerCommand("FullNameCommandPlugin", "command_with_underscore",
                                   [](const std::vector<urpg::Value>& args) -> urpg::Value {
                                       return urpg::Value::Int(static_cast<int64_t>(args.size()));
                                   }));

        const urpg::Value result =
            pm.executeCommandByName("FullNameCommandPlugin_command_with_underscore",
                                    {urpg::Value::Int(1), urpg::Value::Int(2), urpg::Value::Int(3)});
        REQUIRE(std::holds_alternative<int64_t>(result.v));
        REQUIRE(std::get<int64_t>(result.v) == 3);

        pm.unregisterPlugin("FullNameCommandPlugin");
    }

    SECTION("Execute command by full name rejects missing plugin segment") {
        const std::string invalidFullName = "_missingPluginSegment";
        const urpg::Value result = pm.executeCommandByName(invalidFullName, {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);
    }

    SECTION("Execute command by full name rejects missing command segment") {
        const std::string invalidFullName = "missingCommandSegment_";
        const urpg::Value result = pm.executeCommandByName(invalidFullName, {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);
    }

    SECTION("Execute non-existent command returns empty") {
        urpg::Value result = pm.executeCommand("NonExistent", "command", {});
        REQUIRE(result.v.index() == 0); // null/empty
    }

    SECTION("Execute command fails when required dependencies are missing") {
        PluginInfo core;
        core.name = "ExecDepCore";

        PluginInfo dependent;
        dependent.name = "ExecDependentPlugin";
        dependent.dependencies = {"ExecDepCore"};

        REQUIRE(pm.registerPlugin(core));
        REQUIRE(pm.registerPlugin(dependent));

        bool wasExecuted = false;
        REQUIRE(pm.registerCommand("ExecDependentPlugin", "run",
                                   [&wasExecuted](const std::vector<urpg::Value>&) -> urpg::Value {
                                       wasExecuted = true;
                                       return urpg::Value::Int(99);
                                   }));

        REQUIRE(pm.unloadPlugin("ExecDepCore"));

        const urpg::Value result = pm.executeCommand("ExecDependentPlugin", "run", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE_FALSE(wasExecuted);
        REQUIRE(pm.getLastError() == "Missing dependencies for ExecDependentPlugin_run: ExecDepCore");

        pm.unregisterPlugin("ExecDependentPlugin");
    }

    SECTION("Execute command asynchronously with FIFO callback order after main-thread dispatch") {
        pm.registerPlugin(makePluginInfo("AsyncPlugin"));
        pm.registerCommand("AsyncPlugin", "echoInt", [](const std::vector<urpg::Value>& args) -> urpg::Value {
            if (args.empty()) {
                return urpg::Value::Int(-1);
            }
            return args.front();
        });

        std::mutex callbackMutex;
        std::condition_variable callbackCv;
        std::vector<int64_t> callbackOrder;
        int32_t completed = 0;
        bool callbackTypeMismatch = false;

        for (int32_t i = 0; i < 3; ++i) {
            pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(i)}, [&](const urpg::Value& result) {
                std::lock_guard<std::mutex> lock(callbackMutex);
                if (std::holds_alternative<int64_t>(result.v)) {
                    callbackOrder.push_back(std::get<int64_t>(result.v));
                } else {
                    callbackTypeMismatch = true;
                }
                ++completed;
                callbackCv.notify_one();
            });
        }

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            const bool workerExecutedWithoutDispatch =
                callbackCv.wait_for(lock, std::chrono::milliseconds(150), [&]() { return completed > 0; });
            REQUIRE_FALSE(workerExecutedWithoutDispatch);
        }

        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 3);

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            const bool done = callbackCv.wait_for(lock, std::chrono::seconds(2), [&]() { return completed == 3; });
            REQUIRE(done);
        }

        REQUIRE_FALSE(callbackTypeMismatch);
        REQUIRE(callbackOrder.size() == 3);
        REQUIRE(callbackOrder[0] == 0);
        REQUIRE(callbackOrder[1] == 1);
        REQUIRE(callbackOrder[2] == 2);

        pm.unregisterPlugin("AsyncPlugin");
    }

    SECTION("Async callbacks reject dispatch from non-owning thread") {
        pm.registerPlugin(makePluginInfo("AsyncPlugin"));
        pm.registerCommand("AsyncPlugin", "echoInt", [](const std::vector<urpg::Value>& args) -> urpg::Value {
            if (args.empty()) {
                return urpg::Value::Int(-1);
            }
            return args.front();
        });

        std::mutex callbackMutex;
        std::condition_variable callbackCv;
        int32_t completed = 0;
        int32_t workerDispatchCount = -1;

        pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(42)}, [&](const urpg::Value&) {
            std::lock_guard<std::mutex> lock(callbackMutex);
            ++completed;
            callbackCv.notify_one();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        std::thread foreignDispatcher([&]() { workerDispatchCount = pm.dispatchPendingAsyncCallbacks(); });
        foreignDispatcher.join();

        REQUIRE(workerDispatchCount == 0);
        REQUIRE(pm.getLastError() == "dispatchPendingAsyncCallbacks must be called on the owning thread");

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            REQUIRE_FALSE(callbackCv.wait_for(lock, std::chrono::milliseconds(150), [&]() { return completed > 0; }));
        }

        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 1);

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            REQUIRE(callbackCv.wait_for(lock, std::chrono::seconds(2), [&]() { return completed == 1; }));
        }

        pm.unregisterPlugin("AsyncPlugin");
    }

    SECTION("Async callbacks remain queued after rejected foreign-thread drain until owning thread dispatches them") {
        pm.registerPlugin(makePluginInfo("AsyncPlugin"));
        pm.registerCommand("AsyncPlugin", "echoInt", [](const std::vector<urpg::Value>& args) -> urpg::Value {
            if (args.empty()) {
                return urpg::Value::Int(-1);
            }
            return args.front();
        });

        std::mutex callbackMutex;
        std::condition_variable callbackCv;
        std::vector<int64_t> callbackValues;
        bool callbackTypeMismatch = false;

        pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(7)}, [&](const urpg::Value& result) {
            std::lock_guard<std::mutex> lock(callbackMutex);
            if (std::holds_alternative<int64_t>(result.v)) {
                callbackValues.push_back(std::get<int64_t>(result.v));
            } else {
                callbackTypeMismatch = true;
            }
            callbackCv.notify_one();
        });
        pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(8)}, [&](const urpg::Value& result) {
            std::lock_guard<std::mutex> lock(callbackMutex);
            if (std::holds_alternative<int64_t>(result.v)) {
                callbackValues.push_back(std::get<int64_t>(result.v));
            } else {
                callbackTypeMismatch = true;
            }
            callbackCv.notify_one();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        int32_t workerDispatchCount = -1;
        std::thread foreignDispatcher([&]() { workerDispatchCount = pm.dispatchPendingAsyncCallbacks(); });
        foreignDispatcher.join();

        REQUIRE(workerDispatchCount == 0);
        REQUIRE(pm.getLastError() == "dispatchPendingAsyncCallbacks must be called on the owning thread");

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            REQUIRE_FALSE(
                callbackCv.wait_for(lock, std::chrono::milliseconds(150), [&]() { return !callbackValues.empty(); }));
        }

        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 2);

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            REQUIRE(callbackCv.wait_for(lock, std::chrono::seconds(2), [&]() { return callbackValues.size() == 2; }));
        }

        REQUIRE_FALSE(callbackTypeMismatch);
        REQUIRE(callbackValues.size() == 2);
        REQUIRE(callbackValues[0] == 7);
        REQUIRE(callbackValues[1] == 8);

        pm.unregisterPlugin("AsyncPlugin");
    }

    SECTION("Async callback dispatch clears stale foreign-thread error after successful owning-thread drain") {
        pm.registerPlugin(makePluginInfo("AsyncPlugin"));
        pm.registerCommand("AsyncPlugin", "echoInt", [](const std::vector<urpg::Value>& args) -> urpg::Value {
            if (args.empty()) {
                return urpg::Value::Int(-1);
            }
            return args.front();
        });

        std::mutex callbackMutex;
        std::condition_variable callbackCv;
        int32_t completed = 0;

        pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(9)}, [&](const urpg::Value&) {
            std::lock_guard<std::mutex> lock(callbackMutex);
            ++completed;
            callbackCv.notify_one();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        std::thread foreignDispatcher([&]() { REQUIRE(pm.dispatchPendingAsyncCallbacks() == 0); });
        foreignDispatcher.join();

        REQUIRE(pm.getLastError() == "dispatchPendingAsyncCallbacks must be called on the owning thread");

        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 1);
        REQUIRE(pm.getLastError().empty());

        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            REQUIRE(callbackCv.wait_for(lock, std::chrono::seconds(2), [&]() { return completed == 1; }));
        }

        pm.unregisterPlugin("AsyncPlugin");
    }

    SECTION("Fixture script command supports invoke and invokeByName chaining") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_invoke_chain_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "InvokeChainFixture",
  "commands": [
    {
      "name": "base",
      "result": 7
    },
    {
      "name": "countArgs",
      "mode": "arg_count"
    },
    {
      "name": "chain",
      "script": [
        {"op": "invoke", "command": "base", "store": "baseResult", "expect": "non_nil"},
        {"op": "invoke", "command": "countArgs", "args": [1, 2, 3], "store": "argCount", "expect": "non_nil"},
        {"op": "invokeByName", "name": {"from": "concat", "parts": [{"from": "pluginName"}, "_base"]}, "store": "byNameResult", "expect": "non_nil"},
        {"op": "invokeByName", "name": "missingQualifiedName", "store": "missingByName"},
        {"op": "returnObject"}
      ]
    }
  ]
})");

        REQUIRE(pm.loadPlugin(fixtureFile));

        const urpg::Value result = pm.executeCommand("InvokeChainFixture", "chain", {});
        REQUIRE(std::holds_alternative<urpg::Object>(result.v));
        const auto& object = std::get<urpg::Object>(result.v);
        REQUIRE(std::holds_alternative<int64_t>(object.at("baseResult").v));
        REQUIRE(std::get<int64_t>(object.at("baseResult").v) == 7);
        REQUIRE(std::holds_alternative<int64_t>(object.at("argCount").v));
        REQUIRE(std::get<int64_t>(object.at("argCount").v) == 3);
        REQUIRE(std::holds_alternative<int64_t>(object.at("byNameResult").v));
        REQUIRE(std::get<int64_t>(object.at("byNameResult").v) == 7);
        REQUIRE(std::holds_alternative<std::monostate>(object.at("missingByName").v));
        REQUIRE(pm.getLastError() == "Invalid command name format: missingQualifiedName");

        const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
        const auto parseFailureCount =
            static_cast<int32_t>(std::count_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command_by_name_parse" &&
                       row.value("command", "") == "missingQualifiedName";
            }));
        REQUIRE(parseFailureCount == 1);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture script command supports rich invoke expectations") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_rich_expect_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
    "name": "RichExpectFixture",
    "commands": [
        {
            "name": "base",
            "result": 7
        },
        {
            "name": "emitTrue",
            "result": true
        },
        {
            "name": "emitFalse",
            "result": false
        },
        {
            "name": "emitNil",
            "result": null
        },
        {
            "name": "chain",
            "script": [
                {"op": "invoke", "command": "base", "store": "baseResult", "expect": {"equals": 7}},
                {"op": "invoke", "command": "emitTrue", "store": "truthyResult", "expect": "truthy"},
                {"op": "invoke", "command": "emitFalse", "store": "falseyResult", "expect": "falsey"},
                {"op": "invoke", "command": "emitNil", "store": "nilResult", "expect": "nil"},
                {"op": "returnObject"}
            ]
        }
    ]
})");

        REQUIRE(pm.loadPlugin(fixtureFile));

        const urpg::Value result = pm.executeCommand("RichExpectFixture", "chain", {});
        REQUIRE(std::holds_alternative<urpg::Object>(result.v));
        const auto& object = std::get<urpg::Object>(result.v);
        REQUIRE(std::holds_alternative<int64_t>(object.at("baseResult").v));
        REQUIRE(std::get<int64_t>(object.at("baseResult").v) == 7);
        REQUIRE(std::holds_alternative<bool>(object.at("truthyResult").v));
        REQUIRE(std::get<bool>(object.at("truthyResult").v));
        REQUIRE(std::holds_alternative<bool>(object.at("falseyResult").v));
        REQUIRE_FALSE(std::get<bool>(object.at("falseyResult").v));
        REQUIRE(std::holds_alternative<std::monostate>(object.at("nilResult").v));
        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture script resolver validation surfaces specific metadata errors") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_resolver_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
    "name": "BadResolverFixture",
    "parameters": {
        "profile": "unit_profile"
    },
    "commands": [
        {
            "name": "badArg",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "arg", "index": "bad"}}
            ]
        },
        {
            "name": "badHasArg",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "hasArg", "index": "bad"}}
            ]
        },
        {
            "name": "badParam",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "param", "name": 7}}
            ]
        },
        {
            "name": "badHasParam",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "hasParam", "name": 7}}
            ]
        },
        {
            "name": "badLocal",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "local", "name": false}}
            ]
        },
        {
            "name": "badArgCount",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "argCount", "index": 0}}
            ]
        },
        {
            "name": "badArgs",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "args", "name": "bad"}}
            ]
        },
        {
            "name": "badParamKeys",
            "script": [
                {"op": "set", "key": "value", "value": {"from": "paramKeys", "index": 0}}
            ]
        }
    ]
})");

        REQUIRE(pm.loadPlugin(fixtureFile));

        const std::vector<std::pair<std::string, std::string>> failureCases = {
            {"badArg", "Host function error: Fixture script resolver arg requires integer index"},
            {"badHasArg", "Host function error: Fixture script resolver hasArg requires integer index"},
            {"badParam", "Host function error: Fixture script resolver param requires string name"},
            {"badHasParam", "Host function error: Fixture script resolver hasParam requires string name"},
            {"badLocal", "Host function error: Fixture script resolver local requires string name"},
            {"badArgCount", "Host function error: Fixture script resolver argCount does not accept field 'index'"},
            {"badArgs", "Host function error: Fixture script resolver args does not accept field 'name'"},
            {"badParamKeys", "Host function error: Fixture script resolver paramKeys does not accept field 'index'"},
        };

        for (const auto& [commandName, expectedMessage] : failureCases) {
            const urpg::Value result = pm.executeCommand("BadResolverFixture", commandName, {});
            REQUIRE(std::holds_alternative<std::monostate>(result.v));
            REQUIRE(pm.getLastError() == expectedMessage);
        }

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("Fixture script resolver arg requires integer index") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver hasArg requires integer index") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver param requires string name") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver hasParam requires string name") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver local requires string name") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver argCount does not accept field 'index'") !=
                std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver args does not accept field 'name'") != std::string::npos);
        REQUIRE(diagnostics.find("Fixture script resolver paramKeys does not accept field 'index'") !=
                std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command can deterministically drop QuickJS context before call") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_context_drop_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "ContextDropFixture",
  "commands": [
    {
      "name": "dropAndCall",
      "entry": "dropAndCallEntry",
      "dropContextBeforeCall": true,
      "js": "// @urpg-export dropAndCallEntry const 1"
    }
  ]
})");

        REQUIRE(pm.loadPlugin(fixtureFile));

        const urpg::Value result = pm.executeCommand("ContextDropFixture", "dropAndCall", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "QuickJS context missing for plugin: ContextDropFixture");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"execute_command_quickjs_context_missing\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"ContextDropFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"dropAndCall\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command rejects non-boolean dropContextBeforeCall metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_context_drop_flag_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadContextDropFlagFixture",
  "commands": [
    {
      "name": "badDropFlag",
      "entry": "badDropFlagEntry",
      "dropContextBeforeCall": "yes",
      "js": "// @urpg-export badDropFlagEntry const 1"
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_drop_context_flag\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadContextDropFlagFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badDropFlag\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command rejects non-string JS entry metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_entry_type_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture JS command requires string 'entry' payload: badEntry");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_js_entry\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadEntryTypeFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badEntry\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command rejects non-string description metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_description_type_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadDescriptionTypeFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture command 'description' must be string: badDescription");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_description\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDescriptionTypeFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badDescription\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command rejects non-string mode metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_mode_type_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadModeTypeFixture",
  "commands": [
    {
      "name": "badMode",
      "mode": 7,
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture command 'mode' must be string: badMode");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_mode\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadModeTypeFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badMode\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture command rejects unsupported mode metadata value") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_mode_value_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadModeValueFixture",
  "commands": [
    {
      "name": "badModeValue",
      "mode": "unknown_mode",
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture command 'mode' unsupported: unknown_mode for badModeValue");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_mode\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadModeValueFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badModeValue\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture script command reports deterministic QuickJS register-function failure") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_register_script_fn_failure_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "RegisterScriptFnFailureFixture",
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
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            "Failed to register QuickJS fixture function for command: __urpg_fail_register_function___scriptCommand");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_register_script_fn\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"RegisterScriptFnFailureFixture\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"__urpg_fail_register_function___scriptCommand\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture plugin rejects non-array dependencies metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_dependencies_shape_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadDependenciesShapeFixture",
  "dependencies": "CorePlugin",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == ("Fixture plugin 'dependencies' must be array: " + fixtureFile));

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_dependencies\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDependenciesShapeFixture\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture plugin rejects non-string dependency entries") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_dependency_entry_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadDependencyEntryFixture",
  "dependencies": ["CorePlugin", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == ("Fixture plugin dependency must be string at index 1: " + fixtureFile));

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_dependency_entry\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDependencyEntryFixture\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture plugin rejects non-object parameters metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_parameters_shape_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == ("Fixture plugin 'parameters' must be object: " + fixtureFile));

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_parameters\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadParametersShapeFixture\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture plugin rejects non-array commands metadata") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_bad_commands_shape_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(fixtureFile,
                      R"({
  "name": "BadCommandsShapeFixture",
  "commands": {
    "name": "not-an-array"
  }
})");

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == ("Fixture plugin 'commands' must be array: " + fixtureFile));

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_commands\"") != std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadCommandsShapeFixture\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Directory loader can deterministically surface scan iterator failures") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixtureDir = uniqueTempDirectoryPath("urpg___urpg_fail_directory_scan___fixture_dir");
        REQUIRE(std::filesystem::create_directories(fixtureDir));

        const auto loadedCount = pm.loadPluginsFromDirectory(fixtureDir.string());
        REQUIRE(loadedCount == 0);
        REQUIRE(pm.getLastError() == "Failed scanning plugin directory: " + fixtureDir.string());

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugins_directory_scan\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove_all(fixtureDir, ec);
    }

    SECTION("Directory loader can deterministically surface entry status failures") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixtureDir = uniqueTempDirectoryPath("urpg_directory_entry_status_fixture_dir");
        REQUIRE(std::filesystem::create_directories(fixtureDir));

        const auto markerEntry = fixtureDir / "__urpg_fail_directory_entry_status___marker.json";
        writeTextFile(markerEntry, "{}");

        const auto loadedCount = pm.loadPluginsFromDirectory(fixtureDir.string());
        REQUIRE(loadedCount == 0);
        REQUIRE(pm.getLastError().find("Failed reading plugin directory entry:") == 0);
        REQUIRE(pm.getLastError().find("__urpg_fail_directory_entry_status___marker.json") != std::string::npos);

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugins_directory_scan_entry\"") != std::string::npos);
        REQUIRE(diagnostics.find("__urpg_fail_directory_entry_status___marker.json") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove_all(fixtureDir, ec);
    }
}

