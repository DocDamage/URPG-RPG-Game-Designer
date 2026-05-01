#include "tests/compat/compat_plugin_failure_diagnostics_helpers.h"

TEST_CASE("Compat fixtures: runtime QuickJS command failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto runtimeFailureFixture = uniqueTempFixturePath("urpg_runtime_failure_fixture");
    writeTextFile(runtimeFailureFixture,
                  R"({
  "name": "BrokenRuntimeFixture",
  "commands": [
    {
      "name": "brokenRuntime",
      "entry": "brokenRuntimeEntry",
      "js": "// @urpg-fail-call brokenRuntimeEntry fixture runtime call failure"
    }
  ]
})");

    REQUIRE(pm.loadPlugin(runtimeFailureFixture.string()));

    const urpg::Value result = pm.executeCommand("BrokenRuntimeFixture", "brokenRuntime", {});
    REQUIRE(std::holds_alternative<std::monostate>(result.v));
    REQUIRE(pm.getLastError() == "Host function error: fixture runtime call failure");

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& runtimeFailureRow = diagnostics.back();
    REQUIRE(runtimeFailureRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(runtimeFailureRow.value("plugin", "") == "BrokenRuntimeFixture");
    REQUIRE(runtimeFailureRow.value("command", "") == "brokenRuntime");
    REQUIRE(runtimeFailureRow.value("severity", "") == "HARD_FAIL");
    REQUIRE(runtimeFailureRow.value("message", "") == "Host function error: fixture runtime call failure");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(runtimeFailureFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script runtime op failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto runtimeScriptFailureFixture = uniqueTempFixturePath("urpg_runtime_script_failure_fixture");
    writeTextFile(runtimeScriptFailureFixture,
                  R"({
  "name": "BrokenScriptRuntimeFixture",
  "commands": [
    {
      "name": "scriptError",
      "script": [
        {"op": "set", "key": "command", "value": {"from": "commandName"}},
        {"op": "if",
         "condition": {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 1},
         "then": [
           {"op": "error", "message": {"from": "concat", "parts": ["script runtime failure: ", {"from": "local", "name": "command"}]}}
         ],
         "else": [
           {"op": "return", "value": "ok"}
         ]
        }
      ]
    },
    {
      "name": "unsupportedOp",
      "script": [
        {"op": "set", "key": "marker", "value": "start"},
        {"op": "unknown"}
      ]
    }
  ]
})");

    REQUIRE(pm.loadPlugin(runtimeScriptFailureFixture.string()));

    const urpg::Value scriptErrorResult =
        pm.executeCommand("BrokenScriptRuntimeFixture", "scriptError", {urpg::Value::Int(1)});
    REQUIRE(std::holds_alternative<std::monostate>(scriptErrorResult.v));
    REQUIRE(pm.getLastError() == "Host function error: script runtime failure: scriptError");

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& scriptErrorRow = diagnostics.back();
    REQUIRE(scriptErrorRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(scriptErrorRow.value("plugin", "") == "BrokenScriptRuntimeFixture");
    REQUIRE(scriptErrorRow.value("command", "") == "scriptError");
    REQUIRE(scriptErrorRow.value("message", "") == "Host function error: script runtime failure: scriptError");

    const urpg::Value unsupportedOpResult = pm.executeCommand("BrokenScriptRuntimeFixture", "unsupportedOp", {});
    REQUIRE(std::holds_alternative<std::monostate>(unsupportedOpResult.v));
    REQUIRE(pm.getLastError().find("Host function error: Unsupported fixture script op") != std::string::npos);

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& unsupportedOpRow = diagnostics.back();
    REQUIRE(unsupportedOpRow.value("operation", "") == "execute_command_quickjs_call");
    REQUIRE(unsupportedOpRow.value("plugin", "") == "BrokenScriptRuntimeFixture");
    REQUIRE(unsupportedOpRow.value("command", "") == "unsupportedOp");
    REQUIRE(unsupportedOpRow.value("message", "") ==
            "Host function error: Unsupported fixture script op 'unknown' at index 1");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(runtimeScriptFailureFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script invoke command-chain failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto invokeChainFailureFixture = uniqueTempFixturePath("urpg_runtime_invoke_chain_failure_fixture");
    writeTextFile(invokeChainFailureFixture,
                  R"({
  "name": "BrokenInvokeChainFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "invokeMissingRequired",
      "script": [
        {"op": "invoke", "command": "missing", "expect": "non_nil"}
      ]
    },
    {
      "name": "invokeByNameMissingRequired",
      "script": [
        {"op": "invokeByName", "name": "missingQualifiedName", "expect": "non_nil"}
      ]
    },
    {
      "name": "invokeMalformedArgs",
      "script": [
        {"op": "invoke", "command": "seed", "args": {"bad": "shape"}}
      ]
    },
    {
      "name": "invokeMalformedExpect",
      "script": [
        {"op": "invoke", "command": "seed", "expect": "unknown_expect"}
      ]
    },
    {
      "name": "invokeMalformedExpectObject",
      "script": [
        {"op": "invoke", "command": "seed", "expect": {"unsupported": true}}
      ]
    }
  ]
})");

    REQUIRE(pm.loadPlugin(invokeChainFailureFixture.string()));

    const urpg::Value invokeMissingRequiredResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMissingRequiredResult.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script invoke op expected non-nil result for "
                                 "BrokenInvokeChainFixture_missing at index 0");

    const urpg::Value invokeByNameMissingRequiredResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeByNameMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeByNameMissingRequiredResult.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script invokeByName op expected non-nil result for "
                                 "missingQualifiedName at index 0");

    const urpg::Value invokeMalformedArgsResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedArgs", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedArgsResult.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script invoke op requires array args at index 0");

    const urpg::Value invokeMalformedExpectResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedExpect", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedExpectResult.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script invoke op unsupported expect value "
                                 "'unknown_expect' at index 0");

    const urpg::Value invokeMalformedExpectObjectResult =
        pm.executeCommand("BrokenInvokeChainFixture", "invokeMalformedExpectObject", {});
    REQUIRE(std::holds_alternative<std::monostate>(invokeMalformedExpectObjectResult.v));
    REQUIRE(pm.getLastError() ==
            "Host function error: Fixture script invoke op requires supported expect object at index 0");

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());

    const auto invokeMissingRow = std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
        return row.value("operation", "") == "execute_command" &&
               row.value("plugin", "") == "BrokenInvokeChainFixture" && row.value("command", "") == "missing";
    });
    REQUIRE(invokeMissingRow != diagnostics.end());
    REQUIRE(invokeMissingRow->value("severity", "") == "WARN");

    const auto invokeByNameParseRow =
        std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_by_name_parse" &&
                   row.value("command", "") == "missingQualifiedName";
        });
    REQUIRE(invokeByNameParseRow != diagnostics.end());
    REQUIRE(invokeByNameParseRow->value("severity", "") == "WARN");

    const auto invokeRequiredRow = std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
        return row.value("operation", "") == "execute_command_quickjs_call" &&
               row.value("plugin", "") == "BrokenInvokeChainFixture" &&
               row.value("command", "") == "invokeMissingRequired" &&
               row.value("message", "") == "Host function error: Fixture script invoke op expected non-nil result for "
                                           "BrokenInvokeChainFixture_missing at index 0";
    });
    REQUIRE(invokeRequiredRow != diagnostics.end());

    const auto invokeByNameRequiredRow =
        std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeByNameMissingRequired" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invokeByName op expected non-nil result for "
                       "missingQualifiedName at index 0";
        });
    REQUIRE(invokeByNameRequiredRow != diagnostics.end());

    const auto invokeMalformedArgsRow =
        std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMalformedArgs" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op requires array args at index 0";
        });
    REQUIRE(invokeMalformedArgsRow != diagnostics.end());

    const auto invokeMalformedExpectRow =
        std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMalformedExpect" &&
                   row.value("message", "") == "Host function error: Fixture script invoke op unsupported expect value "
                                               "'unknown_expect' at index 0";
        });
    REQUIRE(invokeMalformedExpectRow != diagnostics.end());

    const auto invokeMalformedExpectObjectRow =
        std::find_if(diagnostics.begin(), diagnostics.end(), [](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenInvokeChainFixture" &&
                   row.value("command", "") == "invokeMalformedExpectObject" &&
                   row.value("message", "") ==
                       "Host function error: Fixture script invoke op requires supported expect object at index 0";
        });
    REQUIRE(invokeMalformedExpectObjectRow != diagnostics.end());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(invokeChainFailureFixture, ec);
}

TEST_CASE("Compat fixtures: fixture script validation failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto scriptValidationFixture = uniqueTempFixturePath("urpg_runtime_script_validation_failure_fixture");
    writeTextFile(scriptValidationFixture,
                  R"({
  "name": "BrokenScriptValidationFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "setMissingKey",
      "script": [
        {"op": "set", "value": 1}
      ]
    },
    {
      "name": "appendMissingKey",
      "script": [
        {"op": "append", "value": 1}
      ]
    },
    {
      "name": "invokeMissingCommand",
      "script": [
        {"op": "invoke"}
      ]
    },
    {
      "name": "invokeNonStringCommand",
      "script": [
        {"op": "invoke", "command": 7}
      ]
    },
    {
      "name": "invokeNonStringPlugin",
      "script": [
        {"op": "invoke", "plugin": 7, "command": "seed"}
      ]
    },
    {
      "name": "invokeByNameMissingName",
      "script": [
        {"op": "invokeByName"}
      ]
    },
    {
      "name": "invokeByNameNonStringName",
      "script": [
        {"op": "invokeByName", "name": 7}
      ]
    },
    {
      "name": "invokeBadStoreType",
      "script": [
        {"op": "invoke", "command": "seed", "store": 7}
      ]
    },
    {
      "name": "invokeBadExpectType",
      "script": [
        {"op": "invoke", "command": "seed", "expect": 7}
      ]
    },
    {
      "name": "errorDefaultMessage",
      "script": [
        {"op": "error"}
      ]
    }
  ]
})");

    REQUIRE(pm.loadPlugin(scriptValidationFixture.string()));

    const std::vector<std::pair<std::string, std::string>> failureCases = {
        {"setMissingKey", "Host function error: Fixture script set op requires key at index 0"},
        {"appendMissingKey", "Host function error: Fixture script append op requires key at index 0"},
        {"invokeMissingCommand", "Host function error: Fixture script invoke op requires command at index 0"},
        {"invokeNonStringCommand", "Host function error: Fixture script invoke op requires string command at index 0"},
        {"invokeNonStringPlugin", "Host function error: Fixture script invoke op requires string plugin at index 0"},
        {"invokeByNameMissingName", "Host function error: Fixture script invokeByName op requires name at index 0"},
        {"invokeByNameNonStringName",
         "Host function error: Fixture script invokeByName op requires string name at index 0"},
        {"invokeBadStoreType", "Host function error: Fixture script invoke op requires string store at index 0"},
        {"invokeBadExpectType",
         "Host function error: Fixture script invoke op requires string or object expect at index 0"},
        {"errorDefaultMessage", "Host function error: Fixture script error op triggered at index 0"},
    };

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const urpg::Value result = pm.executeCommand("BrokenScriptValidationFixture", commandName, {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == expectedMessage);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= failureCases.size());

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const auto rowIt = std::find_if(diagnostics.begin(), diagnostics.end(), [&](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenScriptValidationFixture" &&
                   row.value("command", "") == commandName && row.value("message", "") == expectedMessage;
        });
        REQUIRE(rowIt != diagnostics.end());
    }

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(scriptValidationFixture, ec);
}

TEST_CASE("Compat fixtures: malformed nested-branch fixture script failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto nestedBranchFixture = uniqueTempFixturePath("urpg_runtime_nested_branch_validation_failure_fixture");
    writeTextFile(nestedBranchFixture,
                  R"({
  "name": "BrokenNestedBranchFixture",
  "commands": [
    {
      "name": "seed",
      "result": 1
    },
    {
      "name": "nestedThenBranchShape",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": {"op": "return", "value": "bad_shape"}}
        ]}
      ]
    },
    {
      "name": "nestedElseBranchShape",
      "script": [
        {"op": "if", "condition": false, "else": [
          {"op": "if", "condition": false, "else": {"op": "return", "value": "bad_shape"}}
        ]}
      ]
    },
    {
      "name": "nestedMissingOp",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"value": "missing-op"}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedArgs",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "args": {"bad": "shape"}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedStore",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "store": 7}
        ]}
      ]
    },
    {
      "name": "nestedInvokeMalformedExpectObject",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invoke", "command": "seed", "expect": {"unsupported": true}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMissingName",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "expect": "non_nil"}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMalformedStore",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "name": "BrokenNestedBranchFixture_seed", "store": 7}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameMalformedExpectObject",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "invokeByName", "name": "BrokenNestedBranchFixture_seed", "expect": {"unsupported": true}}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameBadResolverParts",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "invokeByName", "name": {"from": "concat", "parts": {"bad": "shape"}}, "expect": "non_nil"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedInvokeByNameUnknownResolverSource",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "invokeByName", "name": {"from": "unknown_resolver", "value": "seed"}, "expect": "non_nil"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepConcatBadParts",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "append", "key": "trace", "value": {"from": "concat", "parts": {"bad": "shape"}}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepCoalesceBadValues",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "coalesced", "value": {"from": "coalesce", "values": {"bad": "shape"}}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepEqualsMissingRight",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": {"from": "equals", "left": {"from": "arg", "index": 0}}, "then": [
            {"op": "return", "value": "unreachable"}
          ], "else": [
            {"op": "return", "value": "also_unreachable"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgBadIndex",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badArg", "value": {"from": "arg", "index": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepParamBadName",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badParam", "value": {"from": "param", "name": 7}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepLocalBadName",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badLocal", "value": {"from": "local", "name": false}}
          ], "else": [
            {"op": "return", "value": "unused"}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepHasArgBadIndex",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badHasArg", "value": {"from": "hasArg", "index": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepHasParamBadName",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badHasParam", "value": {"from": "hasParam", "name": 7}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgCountUnexpectedField",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badArgCount", "value": {"from": "argCount", "index": 0}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepArgsUnexpectedField",
      "script": [
        {"op": "if", "condition": false, "then": [
          {"op": "return", "value": "skip_else"}
        ], "else": [
          {"op": "if", "condition": true, "then": [
            {"op": "set", "key": "badArgs", "value": {"from": "args", "name": "bad"}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedDeepParamKeysUnexpectedField",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "if", "condition": false, "then": [
            {"op": "return", "value": "skip_then"}
          ], "else": [
            {"op": "set", "key": "badParamKeys", "value": {"from": "paramKeys", "index": 0}}
          ]}
        ]}
      ]
    },
    {
      "name": "nestedSetMissingKey",
      "script": [
        {"op": "if", "condition": true, "then": [
          {"op": "set", "value": 1}
        ]}
      ]
    }
  ]
})");

    REQUIRE(pm.loadPlugin(nestedBranchFixture.string()));

    const std::vector<std::pair<std::string, std::string>> failureCases = {
        {"nestedThenBranchShape", "Host function error: Fixture script if branch must be an array at index 0"},
        {"nestedElseBranchShape", "Host function error: Fixture script if branch must be an array at index 0"},
        {"nestedMissingOp", "Host function error: Fixture script step missing op at index 0"},
        {"nestedInvokeMalformedArgs", "Host function error: Fixture script invoke op requires array args at index 0"},
        {"nestedInvokeMalformedStore",
         "Host function error: Fixture script invoke op requires string store at index 0"},
        {"nestedInvokeMalformedExpectObject",
         "Host function error: Fixture script invoke op requires supported expect object at index 0"},
        {"nestedInvokeByNameMissingName",
         "Host function error: Fixture script invokeByName op requires name at index 0"},
        {"nestedInvokeByNameMalformedStore",
         "Host function error: Fixture script invokeByName op requires string store at index 0"},
        {"nestedInvokeByNameMalformedExpectObject",
         "Host function error: Fixture script invokeByName op requires supported expect object at index 0"},
        {"nestedInvokeByNameBadResolverParts",
         "Host function error: Fixture script resolver concat requires array parts"},
        {"nestedInvokeByNameUnknownResolverSource",
         "Host function error: Fixture script resolver unknown source 'unknown_resolver'"},
        {"nestedDeepConcatBadParts", "Host function error: Fixture script resolver concat requires array parts"},
        {"nestedDeepCoalesceBadValues", "Host function error: Fixture script resolver coalesce requires array values"},
        {"nestedDeepEqualsMissingRight", "Host function error: Fixture script resolver equals requires left and right"},
        {"nestedDeepArgBadIndex", "Host function error: Fixture script resolver arg requires integer index"},
        {"nestedDeepParamBadName", "Host function error: Fixture script resolver param requires string name"},
        {"nestedDeepLocalBadName", "Host function error: Fixture script resolver local requires string name"},
        {"nestedDeepHasArgBadIndex", "Host function error: Fixture script resolver hasArg requires integer index"},
        {"nestedDeepHasParamBadName", "Host function error: Fixture script resolver hasParam requires string name"},
        {"nestedDeepArgCountUnexpectedField",
         "Host function error: Fixture script resolver argCount does not accept field 'index'"},
        {"nestedDeepArgsUnexpectedField",
         "Host function error: Fixture script resolver args does not accept field 'name'"},
        {"nestedDeepParamKeysUnexpectedField",
         "Host function error: Fixture script resolver paramKeys does not accept field 'index'"},
        {"nestedSetMissingKey", "Host function error: Fixture script set op requires key at index 0"},
    };

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const urpg::Value result = pm.executeCommand("BrokenNestedBranchFixture", commandName, {});
        REQUIRE(std::holds_alternative<std::monostate>(result.v));
        REQUIRE(pm.getLastError() == expectedMessage);
    }

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= failureCases.size());

    for (const auto& [commandName, expectedMessage] : failureCases) {
        const auto rowIt = std::find_if(diagnostics.begin(), diagnostics.end(), [&](const nlohmann::json& row) {
            return row.value("operation", "") == "execute_command_quickjs_call" &&
                   row.value("plugin", "") == "BrokenNestedBranchFixture" && row.value("command", "") == commandName &&
                   row.value("message", "") == expectedMessage;
        });
        REQUIRE(rowIt != diagnostics.end());
    }

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(nestedBranchFixture, ec);
}
