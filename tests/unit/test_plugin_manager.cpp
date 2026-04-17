// PluginManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Plugin Command Registry + Execution

#include "runtimes/compat_js/plugin_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>

using namespace urpg::compat;

namespace {

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

std::filesystem::path fixturePath(const std::string& pluginName) {
    std::vector<std::filesystem::path> candidateRoots;
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        candidateRoots.push_back(
            sourceRoot / "tests" / "compat" / "fixtures" / "plugins"
        );
    }
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

std::filesystem::path uniqueTempDirectoryPath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           (std::string(stem) + "_" + std::to_string(ticks));
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

std::vector<nlohmann::json> parseJsonl(std::string_view jsonl) {
    std::vector<nlohmann::json> rows;
    if (jsonl.empty()) {
        return rows;
    }

    std::istringstream stream{std::string(jsonl)};
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto row = nlohmann::json::parse(line, nullptr, false);
        if (!row.is_discarded() && row.is_object()) {
            rows.push_back(std::move(row));
        }
    }
    return rows;
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

    SECTION("Load plugins from directory uses deterministic lexical order") {
        const auto tempDir = uniqueTempDirectoryPath("urpg_plugin_order");
        REQUIRE(std::filesystem::create_directories(tempDir));

        const auto pluginCPath = tempDir / "c_fixture.json";
        const auto pluginAPath = tempDir / "a_fixture.json";
        const auto pluginBPath = tempDir / "b_fixture.json";

        writeTextFile(
            pluginCPath,
            R"({"name":"OrderPluginC","commands":[{"name":"probe","mode":"arg_count"}]})"
        );
        writeTextFile(
            pluginAPath,
            R"({"name":"OrderPluginA","commands":[{"name":"probe","mode":"arg_count"}]})"
        );
        writeTextFile(
            pluginBPath,
            R"({"name":"OrderPluginB","commands":[{"name":"probe","mode":"arg_count"}]})"
        );

        std::vector<std::string> loadOrder;
        const int32_t handlerId = pm.registerEventHandler(
            PluginManager::PluginEvent::ON_LOAD,
            [&loadOrder](const std::string& pluginName, PluginManager::PluginEvent) {
                loadOrder.push_back(pluginName);
            }
        );

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

    SECTION("Get plugin commands returns deterministic lexical order") {
        pm.registerPlugin({"SortedCommandPlugin", "1.0", "", ""});

        pm.registerCommand("SortedCommandPlugin", "zeta", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("SortedCommandPlugin", "alpha", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });
        pm.registerCommand("SortedCommandPlugin", "middle", [](const std::vector<urpg::Value>&) -> urpg::Value { return {}; });

        const auto commands = pm.getPluginCommands("SortedCommandPlugin");
        REQUIRE(commands.size() == 3);
        REQUIRE(commands[0] == "alpha");
        REQUIRE(commands[1] == "middle");
        REQUIRE(commands[2] == "zeta");

        pm.unregisterPlugin("SortedCommandPlugin");
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

    SECTION("Execute command by full name supports commands with underscores") {
        REQUIRE(pm.registerPlugin({"FullNameCommandPlugin", "1.0", "", ""}));
        REQUIRE(pm.registerCommand(
            "FullNameCommandPlugin",
            "command_with_underscore",
            [](const std::vector<urpg::Value>& args) -> urpg::Value {
                return urpg::Value::Int(static_cast<int64_t>(args.size()));
            }
        ));

        const urpg::Value result = pm.executeCommandByName(
            "FullNameCommandPlugin_command_with_underscore",
            {urpg::Value::Int(1), urpg::Value::Int(2), urpg::Value::Int(3)}
        );
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
        REQUIRE(pm.registerCommand(
            "ExecDependentPlugin",
            "run",
            [&wasExecuted](const std::vector<urpg::Value>&) -> urpg::Value {
                wasExecuted = true;
                return urpg::Value::Int(99);
            }
        ));

        REQUIRE(pm.unloadPlugin("ExecDepCore"));

        const urpg::Value result = pm.executeCommand("ExecDependentPlugin", "run", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE_FALSE(wasExecuted);
        REQUIRE(
            pm.getLastError() ==
            "Missing dependencies for ExecDependentPlugin_run: ExecDepCore"
        );

        pm.unregisterPlugin("ExecDependentPlugin");
    }

    SECTION("Execute command asynchronously with FIFO callback order after main-thread dispatch") {
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
            const bool workerExecutedWithoutDispatch = callbackCv.wait_for(
                lock,
                std::chrono::milliseconds(150),
                [&]() { return completed > 0; }
            );
            REQUIRE_FALSE(workerExecutedWithoutDispatch);
        }

        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 3);

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

    SECTION("Fixture script command supports invoke and invokeByName chaining") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath = uniqueTempDirectoryPath("urpg_invoke_chain_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(
            fixtureFile,
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
})"
        );

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
        const auto parseFailureCount = static_cast<int32_t>(std::count_if(
            diagnostics.begin(),
            diagnostics.end(),
            [](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command_by_name_parse" &&
                       row.value("command", "") == "missingQualifiedName";
            }
        ));
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
                writeTextFile(
                        fixtureFile,
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
})"
                );

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
                writeTextFile(
                        fixtureFile,
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
})"
                );

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
                REQUIRE(diagnostics.find("Fixture script resolver argCount does not accept field 'index'") != std::string::npos);
                REQUIRE(diagnostics.find("Fixture script resolver args does not accept field 'name'") != std::string::npos);
                REQUIRE(diagnostics.find("Fixture script resolver paramKeys does not accept field 'index'") != std::string::npos);

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
        writeTextFile(
            fixtureFile,
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
})"
        );

        REQUIRE(pm.loadPlugin(fixtureFile));

        const urpg::Value result = pm.executeCommand("ContextDropFixture", "dropAndCall", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "QuickJS context missing for plugin: ContextDropFixture");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"execute_command_quickjs_context_missing\"") !=
                std::string::npos);
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
        writeTextFile(
            fixtureFile,
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
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag"
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_drop_context_flag\"") !=
                std::string::npos);
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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})"
        );

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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadDescriptionTypeFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            "Fixture command 'description' must be string: badDescription"
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_description\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDescriptionTypeFixture\"") !=
                std::string::npos);
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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadModeTypeFixture",
  "commands": [
    {
      "name": "badMode",
      "mode": 7,
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(pm.getLastError() == "Fixture command 'mode' must be string: badMode");

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_mode\"") !=
                std::string::npos);
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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadModeValueFixture",
  "commands": [
    {
      "name": "badModeValue",
      "mode": "unknown_mode",
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            "Fixture command 'mode' unsupported: unknown_mode for badModeValue"
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_command_mode\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadModeValueFixture\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"badModeValue\"") != std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Fixture script command reports deterministic QuickJS register-function failure") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixturePath =
            uniqueTempDirectoryPath("urpg_register_script_fn_failure_fixture");
        const auto fixtureFile = fixturePath.string() + ".json";
        writeTextFile(
            fixtureFile,
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
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            "Failed to register QuickJS fixture function for command: __urpg_fail_register_function___scriptCommand"
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_register_script_fn\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"RegisterScriptFnFailureFixture\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"command\":\"__urpg_fail_register_function___scriptCommand\"") !=
                std::string::npos);

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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadDependenciesShapeFixture",
  "dependencies": "CorePlugin",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            ("Fixture plugin 'dependencies' must be array: " + fixtureFile)
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_dependencies\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDependenciesShapeFixture\"") !=
                std::string::npos);

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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadDependencyEntryFixture",
  "dependencies": ["CorePlugin", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            ("Fixture plugin dependency must be string at index 1: " + fixtureFile)
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_dependency_entry\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadDependencyEntryFixture\"") !=
                std::string::npos);

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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            ("Fixture plugin 'parameters' must be object: " + fixtureFile)
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_parameters\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadParametersShapeFixture\"") !=
                std::string::npos);

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
        writeTextFile(
            fixtureFile,
            R"({
  "name": "BadCommandsShapeFixture",
  "commands": {
    "name": "not-an-array"
  }
})"
        );

        REQUIRE_FALSE(pm.loadPlugin(fixtureFile));
        REQUIRE(
            pm.getLastError() ==
            ("Fixture plugin 'commands' must be array: " + fixtureFile)
        );

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugin_commands\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("\"plugin\":\"BadCommandsShapeFixture\"") !=
                std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(fixtureFile, ec);
    }

    SECTION("Directory loader can deterministically surface scan iterator failures") {
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        const auto fixtureDir =
            uniqueTempDirectoryPath("urpg___urpg_fail_directory_scan___fixture_dir");
        REQUIRE(std::filesystem::create_directories(fixtureDir));

        const auto loadedCount = pm.loadPluginsFromDirectory(fixtureDir.string());
        REQUIRE(loadedCount == 0);
        REQUIRE(pm.getLastError() == "Failed scanning plugin directory: " + fixtureDir.string());

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugins_directory_scan\"") !=
                std::string::npos);

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

        const auto markerEntry =
            fixtureDir / "__urpg_fail_directory_entry_status___marker.json";
        writeTextFile(markerEntry, "{}");

        const auto loadedCount = pm.loadPluginsFromDirectory(fixtureDir.string());
        REQUIRE(loadedCount == 0);
        REQUIRE(pm.getLastError().find("Failed reading plugin directory entry:") == 0);
        REQUIRE(pm.getLastError().find("__urpg_fail_directory_entry_status___marker.json") !=
                std::string::npos);

        const std::string diagnostics = pm.exportFailureDiagnosticsJsonl();
        REQUIRE(diagnostics.find("\"operation\":\"load_plugins_directory_scan_entry\"") !=
                std::string::npos);
        REQUIRE(diagnostics.find("__urpg_fail_directory_entry_status___marker.json") !=
                std::string::npos);

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove_all(fixtureDir, ec);
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

    SECTION("Unknown command exceptions are tagged as crash prevented diagnostics") {
        pm.clearFailureDiagnostics();
        REQUIRE(pm.registerPlugin({"CrashPreventedPlugin", "1.0", "", ""}));
        REQUIRE(pm.registerCommand(
            "CrashPreventedPlugin",
            "throwUnknown",
            [](const std::vector<urpg::Value>&) -> urpg::Value {
                throw 7;
            }
        ));

        const urpg::Value result = pm.executeCommand("CrashPreventedPlugin", "throwUnknown", {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == "Unknown command execution error");

        const auto rows = parseJsonl(pm.exportFailureDiagnosticsJsonl());
        REQUIRE_FALSE(rows.empty());
        const auto rowIt = std::find_if(
            rows.begin(),
            rows.end(),
            [](const nlohmann::json& row) {
                return row.value("operation", "") == "execute_command" &&
                       row.value("plugin", "") == "CrashPreventedPlugin" &&
                       row.value("command", "") == "throwUnknown";
            }
        );
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
    
    SECTION("GetMethodStatus returns PARTIAL for async execution") {
        CompatStatus status = pm.getMethodStatus("executeCommandAsync");
        REQUIRE(status == CompatStatus::PARTIAL);
        REQUIRE(pm.getMethodDeviation("executeCommandAsync").find("main-thread dispatch") != std::string::npos);
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
