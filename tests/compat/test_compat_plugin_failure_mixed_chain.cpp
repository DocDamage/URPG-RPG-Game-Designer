#include "compat_plugin_failure_mixed_chain_helpers.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <variant>

namespace {

using urpg::compat::PluginManager;
using namespace urpg::tests::compat_mixed_chain;

} // namespace

TEST_CASE(
    "Compat fixtures: combined weekly regression covers curated dependency gating and mixed malformed/runtime chains",
    "[compat][fixtures][failure][weekly]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto& specs = fixtureSpecs();
    loadCuratedFixturePluginsAndVerifyHappyPaths(pm);
    registerDependencyGateProbesAndVerifyBlocking(pm);
    const auto weeklyLifecycleFixture = createAndExerciseWeeklyLifecycleFixture(pm);

    const auto malformedFixture = uniqueTempFixturePath("urpg_mixed_chain_malformed_fixture");
    writeTextFile(malformedFixture, "{\"name\":");
    REQUIRE_FALSE(pm.loadPlugin(malformedFixture.string()));

    const auto missingFixture = uniqueTempFixturePath("urpg_mixed_chain_missing_fixture");
    std::filesystem::create_directories(missingFixture);
    REQUIRE(std::filesystem::exists(missingFixture));
    REQUIRE(std::filesystem::is_directory(missingFixture));
    REQUIRE(missingFixture.extension() == ".json");
    REQUIRE_FALSE(pm.loadPlugin(missingFixture.string()));
    REQUIRE(pm.getLastError().find("Failed to open plugin fixture:") != std::string::npos);

    const auto scanFailureDir =
        uniqueTempFixturePath("urpg_mixed_chain___urpg_fail_directory_scan___fixture_dir");
    std::filesystem::create_directories(scanFailureDir);
    REQUIRE(std::filesystem::exists(scanFailureDir));
    REQUIRE(std::filesystem::is_directory(scanFailureDir));
    REQUIRE(pm.loadPluginsFromDirectory(scanFailureDir.string()) == 0);
    REQUIRE(pm.getLastError().find("Failed scanning plugin directory:") != std::string::npos);

    const auto scanEntryFailureDir =
        uniqueTempFixturePath("urpg_mixed_chain_directory_entry_status_fixture_dir");
    std::filesystem::create_directories(scanEntryFailureDir);
    const auto scanEntryMarker =
        scanEntryFailureDir / "__urpg_fail_directory_entry_status___marker.json";
    writeTextFile(scanEntryMarker, "{}");
    REQUIRE(pm.loadPluginsFromDirectory(scanEntryFailureDir.string()) == 0);
    REQUIRE(pm.getLastError().find("Failed reading plugin directory entry:") !=
            std::string::npos);
    REQUIRE(pm.getLastError().find("__urpg_fail_directory_entry_status___marker.json") !=
            std::string::npos);

    const auto duplicateFixture = uniqueTempFixturePath("urpg_mixed_chain_duplicate_fixture");
    writeTextFile(
        duplicateFixture,
        R"({
  "name": "MixedChainDuplicateFixture",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE(pm.loadPlugin(duplicateFixture.string()));
    REQUIRE_FALSE(pm.loadPlugin(duplicateFixture.string()));
    REQUIRE(pm.getLastError() == "Plugin already registered: MixedChainDuplicateFixture");

    const auto emptyNameFixture = uniqueTempFixturePath("urpg_mixed_chain_empty_name_fixture");
    writeTextFile(
        emptyNameFixture,
        R"({
  "name": "",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(emptyNameFixture.string()));
    REQUIRE(pm.getLastError().find("Fixture plugin name cannot be empty:") != std::string::npos);

    REQUIRE_FALSE(pm.loadPlugin(""));
    REQUIRE(pm.getLastError() == "Plugin name cannot be empty");

    const auto duplicateCommandFixture =
        uniqueTempFixturePath("urpg_mixed_chain_register_command_fixture");
    writeTextFile(
        duplicateCommandFixture,
        R"({
  "name": "MixedChainRegisterCommandFixture",
  "commands": [
    {
      "name": "dup",
      "result": 1
    },
    {
      "name": "dup",
      "result": 2
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(duplicateCommandFixture.string()));
    REQUIRE(pm.getLastError() == "Command already registered: MixedChainRegisterCommandFixture_dup");

    const auto contextInitFailFixture =
        uniqueTempFixturePath("urpg_mixed_chain_context_init_fail_fixture");
    writeTextFile(
        contextInitFailFixture,
        R"({
  "name": "MixedChain__urpg_fail_context_init__Fixture",
  "commands": [
    {
      "name": "ctxInitFail",
      "js": "// @urpg-export ctxInitFail const 1"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(contextInitFailFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Failed to initialize QuickJS context for plugin: MixedChain__urpg_fail_context_init__Fixture"
    );

    const auto registerScriptFnFailureFixture =
        uniqueTempFixturePath("urpg_mixed_chain_register_script_fn_fixture");
    writeTextFile(
        registerScriptFnFailureFixture,
        R"({
  "name": "MixedChainRegisterScriptFnFixture",
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
    REQUIRE_FALSE(pm.loadPlugin(registerScriptFnFailureFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Failed to register QuickJS fixture function for command: __urpg_fail_register_function___scriptCommand"
    );

    REQUIRE_FALSE(pm.parseParameters("", R"({"k":"v"})"));
    REQUIRE(pm.getLastError() == "Plugin name cannot be empty");

    REQUIRE_FALSE(pm.parseParameters("MixedChainParamsFixture", "[]"));
    REQUIRE(pm.getLastError() == "Parameter JSON must be a valid object");

    const auto dependencyShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_dependency_shape_fixture");
    writeTextFile(
        dependencyShapeFixture,
        R"({
  "name": "MixedChainDependencyShapeFixture",
  "dependencies": "VisuStella_CoreEngine_MZ",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(dependencyShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'dependencies' must be array: " + dependencyShapeFixture.string())
    );

    const auto dependencyEntryFixture =
        uniqueTempFixturePath("urpg_mixed_chain_dependency_entry_fixture");
    writeTextFile(
        dependencyEntryFixture,
        R"({
  "name": "MixedChainDependencyEntryFixture",
  "dependencies": ["VisuStella_CoreEngine_MZ", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(dependencyEntryFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin dependency must be string at index 1: " +
         dependencyEntryFixture.string())
    );

    const auto parametersShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_parameters_shape_fixture");
    writeTextFile(
        parametersShapeFixture,
        R"({
  "name": "MixedChainParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(parametersShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'parameters' must be object: " + parametersShapeFixture.string())
    );

    const auto commandsShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_commands_shape_fixture");
    writeTextFile(
        commandsShapeFixture,
        R"({
  "name": "MixedChainCommandsShapeFixture",
  "commands": {
    "name": "bad-shape"
  }
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandsShapeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        ("Fixture plugin 'commands' must be array: " + commandsShapeFixture.string())
    );

    const auto payloadFixture = uniqueTempFixturePath("urpg_mixed_chain_payload_fixture");
    writeTextFile(
        payloadFixture,
        R"({
  "name": "MixedChainPayloadFixture",
  "commands": [
    {
      "name": "badPayload",
      "js": 123
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(payloadFixture.string()));

    const auto scriptPayloadFixture =
        uniqueTempFixturePath("urpg_mixed_chain_script_payload_fixture");
    writeTextFile(
        scriptPayloadFixture,
        R"({
  "name": "MixedChainScriptPayloadFixture",
  "commands": [
    {
      "name": "badScriptPayload",
      "script": {
        "op": "set"
      }
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(scriptPayloadFixture.string()));

    const auto commandShapeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_shape_fixture");
    writeTextFile(
        commandShapeFixture,
        R"({
  "name": "MixedChainCommandShapeFixture",
  "commands": [
    7
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandShapeFixture.string()));

    const auto commandNameFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_name_fixture");
    writeTextFile(
        commandNameFixture,
        R"({
  "name": "MixedChainCommandNameFixture",
  "commands": [
    {
      "name": "",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandNameFixture.string()));

    const auto dropContextFlagFixture =
        uniqueTempFixturePath("urpg_mixed_chain_drop_context_flag_fixture");
    writeTextFile(
        dropContextFlagFixture,
        R"({
  "name": "MixedChainDropContextFlagFixture",
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
    REQUIRE_FALSE(pm.loadPlugin(dropContextFlagFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag"
    );

    const auto entryTypeFixture = uniqueTempFixturePath("urpg_mixed_chain_entry_type_fixture");
    writeTextFile(
        entryTypeFixture,
        R"({
  "name": "MixedChainEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(entryTypeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'entry' payload: badEntry");

    const auto commandDescriptionFixture =
        uniqueTempFixturePath("urpg_mixed_chain_command_description_fixture");
    writeTextFile(
        commandDescriptionFixture,
        R"({
  "name": "MixedChainCommandDescriptionFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(commandDescriptionFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'description' must be string: badDescription"
    );

    const auto unsupportedModeFixture =
        uniqueTempFixturePath("urpg_mixed_chain_unsupported_mode_fixture");
    writeTextFile(
        unsupportedModeFixture,
        R"({
  "name": "MixedChainUnsupportedModeFixture",
  "commands": [
    {
      "name": "badModeValue",
      "mode": "unknown_mode",
      "result": 1
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(unsupportedModeFixture.string()));
    REQUIRE(
        pm.getLastError() ==
        "Fixture command 'mode' unsupported: unknown_mode for badModeValue"
    );

    const auto evalFixture = uniqueTempFixturePath("urpg_mixed_chain_eval_fixture");
    writeTextFile(
        evalFixture,
        R"({
  "name": "MixedChainEvalFixture",
  "commands": [
    {
      "name": "evalFail",
      "entry": "evalFailEntry",
      "js": "// @urpg-fail-eval mixed chain eval failure"
    }
  ]
})"
    );
    REQUIRE_FALSE(pm.loadPlugin(evalFixture.string()));
    REQUIRE(pm.getLastError() == "mixed chain eval failure");

    const auto runtimeFixture = uniqueTempFixturePath("urpg_mixed_chain_runtime_fixture");
    writeTextFile(
        runtimeFixture,
        R"({
  "name": "MixedChainRuntimeFixture",
  "commands": [
    {
      "name": "runtimeSeed",
      "result": 5
    },
    {
      "name": "runtimeJsFail",
      "entry": "runtimeJsFailEntry",
      "js": "// @urpg-fail-call runtimeJsFailEntry mixed chain js failure"
    },
    {
      "name": "runtimeInvokeChainOk",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "store": "seedResult", "expect": "non_nil"},
        {"op": "invokeByName", "name": {"from": "concat", "parts": [{"from": "pluginName"}, "_runtimeSeed"]}, "store": "seedByName", "expect": "non_nil"},
        {"op": "returnObject"}
      ]
    },
    {
      "name": "runtimeInvokeMissingRequired",
      "script": [
        {"op": "invoke", "command": "runtimeMissingTarget", "expect": "non_nil"}
      ]
    },
    {
      "name": "runtimeInvokeByNameMissingRequired",
      "script": [
        {"op": "invokeByName", "name": "runtimeInvalidByName", "expect": "non_nil"}
      ]
    },
    {
      "name": "runtimeInvokeMalformedArgs",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "args": {"bad": "shape"}}
      ]
    },
    {
      "name": "runtimeInvokeMalformedExpect",
      "script": [
        {"op": "invoke", "command": "runtimeSeed", "expect": "unknown_expect"}
      ]
    },
    {
      "name": "runtimeNestedInvokeMalformedStore",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invoke", "command": "runtimeSeed", "store": 7}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeMalformedExpectObject",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invoke", "command": "runtimeSeed", "expect": {"unsupported": true}}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeByNameMalformedStore",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invokeByName", "name": "MixedChainRuntimeFixture_runtimeSeed", "store": 7}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedInvokeByNameMalformedExpectObject",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {"op": "invokeByName", "name": "MixedChainRuntimeFixture_runtimeSeed", "expect": {"unsupported": true}}
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedInvokeByNameBadResolverParts",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "invokeByName", "name": {"from": "concat", "parts": {"bad": "shape"}}, "expect": "non_nil"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedInvokeByNameUnknownResolverSource",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "invokeByName", "name": {"from": "unknown_resolver", "value": "runtimeSeed"}, "expect": "non_nil"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedConcatBadParts",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "append", "key": "trace", "value": {"from": "concat", "parts": {"bad": "shape"}}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedCoalesceBadValues",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "coalesced", "value": {"from": "coalesce", "values": {"bad": "shape"}}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedEqualsMissingRight",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": {"from": "equals", "left": {"from": "arg", "index": 0}},
              "then": [
                {"op": "return", "value": "unreachable"}
              ],
              "else": [
                {"op": "return", "value": "still_unreachable"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgBadIndex",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badArg", "value": {"from": "arg", "index": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedParamBadName",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badParam", "value": {"from": "param", "name": 7}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedLocalBadName",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badLocal", "value": {"from": "local", "name": false}}
              ],
              "else": [
                {"op": "return", "value": "unused"}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedHasArgBadIndex",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badHasArg", "value": {"from": "hasArg", "index": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedHasParamBadName",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badHasParam", "value": {"from": "hasParam", "name": 7}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgCountUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badArgCount", "value": {"from": "argCount", "index": 0}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedArgsUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": false,
          "then": [
            {"op": "return", "value": "skip_else"}
          ],
          "else": [
            {
              "op": "if",
              "condition": true,
              "then": [
                {"op": "set", "key": "badArgs", "value": {"from": "args", "name": "bad"}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeDeepMixedParamKeysUnexpectedField",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": [
            {
              "op": "if",
              "condition": false,
              "then": [
                {"op": "return", "value": "skip_then"}
              ],
              "else": [
                {"op": "set", "key": "badParamKeys", "value": {"from": "paramKeys", "index": 0}}
              ]
            }
          ]
        }
      ]
    },
    {
      "name": "runtimeScriptUnknown",
      "script": [
        {"op": "set", "key": "stage", "value": "pre"},
        {"op": "unknown"}
      ]
    },
    {
      "name": "runtimeScriptError",
      "script": [
        {"op": "error", "message": "mixed chain script error"}
      ]
    },
    {
      "name": "runtimeNestedAllAnyScriptError",
      "script": [
        {
          "op": "if",
          "condition": {
            "from": "all",
            "values": [
              {"from": "hasArg", "index": 0},
              {
                "from": "any",
                "values": [
                  {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 1},
                  {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 2}
                ]
              }
            ]
          },
          "then": [
            {"op": "error", "message": "mixed chain nested all/any script error"}
          ],
          "else": [
            {"op": "return", "value": "ok"}
          ]
        }
      ]
    },
    {
      "name": "runtimeNestedAllAnyUnknown",
      "script": [
        {
          "op": "if",
          "condition": {
            "from": "any",
            "values": [
              {
                "from": "all",
                "values": [
                  {"from": "hasArg", "index": 0},
                  {"from": "greaterThan", "left": {"from": "arg", "index": 0}, "right": 0}
                ]
              },
              {"from": "equals", "left": {"from": "arg", "index": 0}, "right": 0}
            ]
          },
          "then": [
            {"op": "unknown"}
          ],
          "else": [
            {"op": "return", "value": "ok"}
          ]
        }
      ]
    },
    {
      "name": "runtimeContextMissingViaDrop",
      "entry": "runtimeContextMissingViaDropEntry",
      "dropContextBeforeCall": true,
      "js": "// @urpg-export runtimeContextMissingViaDropEntry const 1"
    },
    {
      "name": "runtimeMissingOp",
      "script": [
        {"value": "missing-op"}
      ]
    },
    {
      "name": "runtimeNonObjectStep",
      "script": [
        7
      ]
    },
    {
      "name": "runtimeIfBranchShape",
      "script": [
        {
          "op": "if",
          "condition": true,
          "then": {
            "op": "return",
            "value": "bad-shape"
          }
        }
      ]
    }
  ]
})"
    );
    REQUIRE(pm.loadPlugin(runtimeFixture.string()));

    const urpg::Value runtimeJsFail =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeJsFail", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeJsFail.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain js failure");

    const urpg::Value runtimeInvokeChainOk =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeChainOk", {});
    REQUIRE(std::holds_alternative<urpg::Object>(runtimeInvokeChainOk.v));
    const auto& runtimeInvokeChainObject = std::get<urpg::Object>(runtimeInvokeChainOk.v);
    REQUIRE(std::holds_alternative<int64_t>(runtimeInvokeChainObject.at("seedResult").v));
    REQUIRE(std::get<int64_t>(runtimeInvokeChainObject.at("seedResult").v) == 5);
    REQUIRE(std::holds_alternative<int64_t>(runtimeInvokeChainObject.at("seedByName").v));
    REQUIRE(std::get<int64_t>(runtimeInvokeChainObject.at("seedByName").v) == 5);

    const urpg::Value runtimeInvokeMissingRequired =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMissingRequired.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op expected non-nil result for "
        "MixedChainRuntimeFixture_runtimeMissingTarget at index 0"
    );

    const urpg::Value runtimeInvokeByNameMissingRequired =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeByNameMissingRequired", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeByNameMissingRequired.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invokeByName op expected non-nil result for "
        "runtimeInvalidByName at index 0"
    );

    const urpg::Value runtimeInvokeMalformedArgs =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMalformedArgs", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMalformedArgs.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op requires array args at index 0"
    );

    const urpg::Value runtimeInvokeMalformedExpect =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeInvokeMalformedExpect", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeInvokeMalformedExpect.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script invoke op unsupported expect value "
        "'unknown_expect' at index 0"
    );

    const urpg::Value runtimeNestedInvokeMalformedStore =
      pm.executeCommand("MixedChainRuntimeFixture", "runtimeNestedInvokeMalformedStore", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeMalformedStore.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invoke op requires string store at index 0"
    );

    const urpg::Value runtimeNestedInvokeMalformedExpectObject =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeMalformedExpectObject",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeMalformedExpectObject.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invoke op requires supported expect object at index 0"
    );

    const urpg::Value runtimeNestedInvokeByNameMalformedStore =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeByNameMalformedStore",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeByNameMalformedStore.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invokeByName op requires string store at index 0"
    );

    const urpg::Value runtimeNestedInvokeByNameMalformedExpectObject =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedInvokeByNameMalformedExpectObject",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedInvokeByNameMalformedExpectObject.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script invokeByName op requires supported expect object at index 0"
    );

    const urpg::Value runtimeDeepMixedInvokeByNameBadResolverParts =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedInvokeByNameBadResolverParts",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedInvokeByNameBadResolverParts.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver concat requires array parts"
    );

    const urpg::Value runtimeDeepMixedInvokeByNameUnknownResolverSource =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedInvokeByNameUnknownResolverSource",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedInvokeByNameUnknownResolverSource.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver unknown source 'unknown_resolver'"
    );

    const urpg::Value runtimeDeepMixedConcatBadParts =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedConcatBadParts",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedConcatBadParts.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver concat requires array parts"
    );

    const urpg::Value runtimeDeepMixedCoalesceBadValues =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedCoalesceBadValues",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedCoalesceBadValues.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver coalesce requires array values"
    );

    const urpg::Value runtimeDeepMixedEqualsMissingRight =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedEqualsMissingRight",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedEqualsMissingRight.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver equals requires left and right"
    );

    const urpg::Value runtimeDeepMixedArgBadIndex =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgBadIndex",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgBadIndex.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver arg requires integer index"
    );

    const urpg::Value runtimeDeepMixedParamBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedParamBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedParamBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver param requires string name"
    );

    const urpg::Value runtimeDeepMixedLocalBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedLocalBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedLocalBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver local requires string name"
    );

    const urpg::Value runtimeDeepMixedHasArgBadIndex =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedHasArgBadIndex",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedHasArgBadIndex.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver hasArg requires integer index"
    );

    const urpg::Value runtimeDeepMixedHasParamBadName =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedHasParamBadName",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedHasParamBadName.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver hasParam requires string name"
    );

    const urpg::Value runtimeDeepMixedArgCountUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgCountUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgCountUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver argCount does not accept field 'index'"
    );

    const urpg::Value runtimeDeepMixedArgsUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedArgsUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedArgsUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver args does not accept field 'name'"
    );

    const urpg::Value runtimeDeepMixedParamKeysUnexpectedField =
      pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeDeepMixedParamKeysUnexpectedField",
        {}
      );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeDeepMixedParamKeysUnexpectedField.v));
    REQUIRE(
      pm.getLastError() ==
      "Host function error: Fixture script resolver paramKeys does not accept field 'index'"
    );

    const urpg::Value runtimeScriptUnknown =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeScriptUnknown", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeScriptUnknown.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Unsupported fixture script op 'unknown' at index 1"
    );

    const urpg::Value runtimeScriptError =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeScriptError", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeScriptError.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain script error");

    const urpg::Value runtimeNestedAllAnyScriptError = pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedAllAnyScriptError",
        {urpg::Value::Int(1)}
    );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedAllAnyScriptError.v));
    REQUIRE(pm.getLastError() == "Host function error: mixed chain nested all/any script error");

    const urpg::Value runtimeNestedAllAnyUnknown = pm.executeCommand(
        "MixedChainRuntimeFixture",
        "runtimeNestedAllAnyUnknown",
        {urpg::Value::Int(2)}
    );
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNestedAllAnyUnknown.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Unsupported fixture script op 'unknown' at index 0"
    );

    const urpg::Value runtimeMissingOp =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeMissingOp", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeMissingOp.v));
    REQUIRE(pm.getLastError() == "Host function error: Fixture script step missing op at index 0");

    const urpg::Value runtimeNonObjectStep =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeNonObjectStep", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeNonObjectStep.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script step must be an object at index 0"
    );

    const urpg::Value runtimeIfBranchShape =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeIfBranchShape", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeIfBranchShape.v));
    REQUIRE(
        pm.getLastError() ==
        "Host function error: Fixture script if branch must be an array at index 0"
    );

    const urpg::Value runtimeContextMissingViaDrop =
        pm.executeCommand("MixedChainRuntimeFixture", "runtimeContextMissingViaDrop", {});
    REQUIRE(std::holds_alternative<std::monostate>(runtimeContextMissingViaDrop.v));
    REQUIRE(pm.getLastError() == "QuickJS context missing for plugin: MixedChainRuntimeFixture");

    const std::string invalidFullName = "mixedChainInvalidFullName";
    const urpg::Value invalidByNameResult = pm.executeCommandByName(invalidFullName, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidByNameResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidFullName);

    const std::string invalidMissingPlugin = "_mixedChainMissingPluginSegment";
    const urpg::Value invalidMissingPluginResult =
        pm.executeCommandByName(invalidMissingPlugin, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidMissingPluginResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidMissingPlugin);

    const std::string invalidMissingCommand = "mixedChainMissingCommandSegment_";
    const urpg::Value invalidMissingCommandResult =
        pm.executeCommandByName(invalidMissingCommand, {});
    REQUIRE(std::holds_alternative<std::monostate>(invalidMissingCommandResult.v));
    REQUIRE(pm.getLastError() == "Invalid command name format: " + invalidMissingCommand);

    const std::string diagnosticsJsonl = pm.exportFailureDiagnosticsJsonl();
    const auto diagnostics = parseJsonl(diagnosticsJsonl);
    REQUIRE_FALSE(diagnostics.empty());

    verifyOperationCounts(diagnostics, specs.size());
    verifyDependencyGateDiagnosticRows(diagnostics, specs);
    verifyRuntimeDiagnosticRows(diagnostics);
    verifyLoadPluginDiagnosticRows(diagnostics, missingFixture, emptyNameFixture);

    verifyCompatReportModelAndPanel(
        pm,
        diagnosticsJsonl,
        specs,
        missingFixture,
        malformedFixture,
        emptyNameFixture
    );

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove_all(missingFixture, ec);
    std::filesystem::remove_all(scanFailureDir, ec);
    std::filesystem::remove_all(scanEntryFailureDir, ec);
    std::filesystem::remove(malformedFixture, ec);
    std::filesystem::remove(duplicateFixture, ec);
    std::filesystem::remove(emptyNameFixture, ec);
    std::filesystem::remove(duplicateCommandFixture, ec);
    std::filesystem::remove(contextInitFailFixture, ec);
    std::filesystem::remove(registerScriptFnFailureFixture, ec);
    std::filesystem::remove(dependencyShapeFixture, ec);
    std::filesystem::remove(dependencyEntryFixture, ec);
    std::filesystem::remove(parametersShapeFixture, ec);
    std::filesystem::remove(commandsShapeFixture, ec);
    std::filesystem::remove(payloadFixture, ec);
    std::filesystem::remove(scriptPayloadFixture, ec);
    std::filesystem::remove(commandShapeFixture, ec);
    std::filesystem::remove(commandNameFixture, ec);
    std::filesystem::remove(dropContextFlagFixture, ec);
    std::filesystem::remove(weeklyLifecycleFixture, ec);
    std::filesystem::remove(entryTypeFixture, ec);
    std::filesystem::remove(commandDescriptionFixture, ec);
    std::filesystem::remove(unsupportedModeFixture, ec);
    std::filesystem::remove(evalFixture, ec);
    std::filesystem::remove(runtimeFixture, ec);
}
