#include "tests/compat/compat_plugin_failure_diagnostics_helpers.h"

TEST_CASE("Compat fixtures: malformed fixture load failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    std::vector<CapturedError> errors;
    pm.setErrorHandler(
        [&errors](const std::string& pluginName, const std::string& commandName, const std::string& error) {
            errors.push_back(CapturedError{pluginName, commandName, error});
        });

    const auto malformedFixture = uniqueTempFixturePath("urpg_malformed_fixture");
    writeTextFile(malformedFixture, "{\"name\":");

    const bool loadedMalformed = pm.loadPlugin(malformedFixture.string());
    REQUIRE_FALSE(loadedMalformed);
    REQUIRE(pm.getLastError().find("Invalid plugin fixture JSON:") != std::string::npos);

    const auto diagnosticsAfterMalformed = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnosticsAfterMalformed.empty());
    const auto& malformedRow = diagnosticsAfterMalformed.back();
    REQUIRE(malformedRow.value("operation", "") == "load_plugin_fixture_parse");
    REQUIRE(malformedRow.value("plugin", "") == malformedFixture.stem().string());

    const auto emptyEntryFixture = uniqueTempFixturePath("urpg_empty_entry_fixture");
    writeTextFile(emptyEntryFixture,
                  R"({
  "name": "BrokenEntryFixture",
  "commands": [
    {
      "name": "brokenCommand",
      "entry": "",
      "js": "// @urpg-export brokenCommand const 1"
    }
  ]
})");

    const bool loadedEmptyEntry = pm.loadPlugin(emptyEntryFixture.string());
    REQUIRE_FALSE(loadedEmptyEntry);
    REQUIRE(pm.getLastError() == "Fixture JS command entry cannot be empty: brokenCommand");

    const auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& emptyEntryRow = diagnostics.back();
    REQUIRE(emptyEntryRow.value("operation", "") == "load_plugin_js_entry");
    REQUIRE(emptyEntryRow.value("plugin", "") == "BrokenEntryFixture");
    REQUIRE(emptyEntryRow.value("command", "") == "brokenCommand");

    REQUIRE(errors.size() >= 2);
    REQUIRE(errors[0].error.find("Invalid plugin fixture JSON:") != std::string::npos);
    REQUIRE(errors[1].pluginName == "BrokenEntryFixture");
    REQUIRE(errors[1].commandName == "brokenCommand");

    const auto evalFailureFixture = uniqueTempFixturePath("urpg_eval_failure_fixture");
    writeTextFile(evalFailureFixture,
                  R"({
  "name": "BrokenEvalFixture",
  "commands": [
    {
      "name": "brokenEval",
      "entry": "brokenEvalRuntime",
      "js": "// @urpg-fail-eval fixture eval failure"
    }
  ]
})");

    const bool loadedEvalFailure = pm.loadPlugin(evalFailureFixture.string());
    REQUIRE_FALSE(loadedEvalFailure);
    REQUIRE(pm.getLastError() == "fixture eval failure");

    const auto diagnosticsAfterEval = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnosticsAfterEval.size() >= 3);
    const auto& evalRow = diagnosticsAfterEval.back();
    REQUIRE(evalRow.value("operation", "") == "load_plugin_js_eval");
    REQUIRE(evalRow.value("plugin", "") == "BrokenEvalFixture");
    REQUIRE(evalRow.value("command", "") == "brokenEval");
    REQUIRE(evalRow.value("message", "") == "fixture eval failure");

    REQUIRE(errors.size() >= 3);
    REQUIRE(errors[2].pluginName == "BrokenEvalFixture");
    REQUIRE(errors[2].commandName == "brokenEval");
    REQUIRE(errors[2].error == "fixture eval failure");

    pm.setErrorHandler(nullptr);
    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(malformedFixture, ec);
    std::filesystem::remove(emptyEntryFixture, ec);
    std::filesystem::remove(evalFailureFixture, ec);
}

TEST_CASE("Compat fixtures: malformed fixture command payload failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto badJsPayloadFixture = uniqueTempFixturePath("urpg_bad_js_payload_fixture");
    writeTextFile(badJsPayloadFixture,
                  R"({
  "name": "BrokenJsPayloadFixture",
  "commands": [
    {
      "name": "brokenJsPayload",
      "js": 123
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badJsPayloadFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'js' payload: brokenJsPayload");

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& badJsRow = diagnostics.back();
    REQUIRE(badJsRow.value("operation", "") == "load_plugin_js_payload");
    REQUIRE(badJsRow.value("plugin", "") == "BrokenJsPayloadFixture");
    REQUIRE(badJsRow.value("command", "") == "brokenJsPayload");

    const auto badScriptPayloadFixture = uniqueTempFixturePath("urpg_bad_script_payload_fixture");
    writeTextFile(badScriptPayloadFixture,
                  R"({
  "name": "BrokenScriptPayloadFixture",
  "commands": [
    {
      "name": "brokenScriptPayload",
      "script": {
        "op": "set"
      }
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badScriptPayloadFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture script command requires array 'script' payload: brokenScriptPayload");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& badScriptRow = diagnostics.back();
    REQUIRE(badScriptRow.value("operation", "") == "load_plugin_script_payload");
    REQUIRE(badScriptRow.value("plugin", "") == "BrokenScriptPayloadFixture");
    REQUIRE(badScriptRow.value("command", "") == "brokenScriptPayload");

    const auto conflictingModeFixture = uniqueTempFixturePath("urpg_conflicting_mode_fixture");
    writeTextFile(conflictingModeFixture,
                  R"({
  "name": "BrokenCommandModeFixture",
  "commands": [
    {
      "name": "brokenMode",
      "entry": "brokenModeRuntime",
      "js": "// @urpg-export brokenModeRuntime const 1",
      "script": [
        {"op": "return", "value": 1}
      ]
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(conflictingModeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture command cannot declare both 'js' and 'script': brokenMode");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 3);
    const auto& conflictingModeRow = diagnostics.back();
    REQUIRE(conflictingModeRow.value("operation", "") == "load_plugin_command_mode");
    REQUIRE(conflictingModeRow.value("plugin", "") == "BrokenCommandModeFixture");
    REQUIRE(conflictingModeRow.value("command", "") == "brokenMode");

    const auto badDropContextFlagFixture = uniqueTempFixturePath("urpg_bad_drop_context_flag_fixture");
    writeTextFile(badDropContextFlagFixture,
                  R"({
  "name": "BrokenDropContextFlagFixture",
  "commands": [
    {
      "name": "badDropFlag",
      "entry": "badDropFlagEntry",
      "dropContextBeforeCall": "true",
      "js": "// @urpg-export badDropFlagEntry const 1"
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badDropContextFlagFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture command 'dropContextBeforeCall' must be boolean: badDropFlag");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 4);
    const auto& badDropFlagRow = diagnostics.back();
    REQUIRE(badDropFlagRow.value("operation", "") == "load_plugin_drop_context_flag");
    REQUIRE(badDropFlagRow.value("plugin", "") == "BrokenDropContextFlagFixture");
    REQUIRE(badDropFlagRow.value("command", "") == "badDropFlag");

    const auto badEntryTypeFixture = uniqueTempFixturePath("urpg_bad_entry_type_fixture");
    writeTextFile(badEntryTypeFixture,
                  R"({
  "name": "BrokenEntryTypeFixture",
  "commands": [
    {
      "name": "badEntry",
      "entry": 7,
      "js": "// @urpg-export badEntry const 1"
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badEntryTypeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture JS command requires string 'entry' payload: badEntry");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 5);
    const auto& badEntryTypeRow = diagnostics.back();
    REQUIRE(badEntryTypeRow.value("operation", "") == "load_plugin_js_entry");
    REQUIRE(badEntryTypeRow.value("plugin", "") == "BrokenEntryTypeFixture");
    REQUIRE(badEntryTypeRow.value("command", "") == "badEntry");

    const auto badDescriptionTypeFixture = uniqueTempFixturePath("urpg_bad_description_type_fixture");
    writeTextFile(badDescriptionTypeFixture,
                  R"({
  "name": "BrokenDescriptionTypeFixture",
  "commands": [
    {
      "name": "badDescription",
      "description": {"text":"bad"},
      "result": 1
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badDescriptionTypeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture command 'description' must be string: badDescription");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 6);
    const auto& badDescriptionTypeRow = diagnostics.back();
    REQUIRE(badDescriptionTypeRow.value("operation", "") == "load_plugin_command_description");
    REQUIRE(badDescriptionTypeRow.value("plugin", "") == "BrokenDescriptionTypeFixture");
    REQUIRE(badDescriptionTypeRow.value("command", "") == "badDescription");

    const auto unsupportedModeFixture = uniqueTempFixturePath("urpg_unsupported_mode_fixture");
    writeTextFile(unsupportedModeFixture,
                  R"({
  "name": "BrokenUnsupportedModeFixture",
  "commands": [
    {
      "name": "badModeValue",
      "mode": "unknown_mode",
      "result": 1
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(unsupportedModeFixture.string()));
    REQUIRE(pm.getLastError() == "Fixture command 'mode' unsupported: unknown_mode for badModeValue");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 7);
    const auto& unsupportedModeRow = diagnostics.back();
    REQUIRE(unsupportedModeRow.value("operation", "") == "load_plugin_command_mode");
    REQUIRE(unsupportedModeRow.value("plugin", "") == "BrokenUnsupportedModeFixture");
    REQUIRE(unsupportedModeRow.value("command", "") == "badModeValue");

    const auto registerScriptFnFailureFixture = uniqueTempFixturePath("urpg_register_script_fn_failure_fixture");
    writeTextFile(registerScriptFnFailureFixture,
                  R"({
  "name": "BrokenRegisterScriptFnFixture",
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

    REQUIRE_FALSE(pm.loadPlugin(registerScriptFnFailureFixture.string()));
    REQUIRE(pm.getLastError() ==
            "Failed to register QuickJS fixture function for command: __urpg_fail_register_function___scriptCommand");

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 8);
    const auto& registerScriptFnRow = diagnostics.back();
    REQUIRE(registerScriptFnRow.value("operation", "") == "load_plugin_register_script_fn");
    REQUIRE(registerScriptFnRow.value("plugin", "") == "BrokenRegisterScriptFnFixture");
    REQUIRE(registerScriptFnRow.value("command", "") == "__urpg_fail_register_function___scriptCommand");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(badJsPayloadFixture, ec);
    std::filesystem::remove(badScriptPayloadFixture, ec);
    std::filesystem::remove(conflictingModeFixture, ec);
    std::filesystem::remove(badDropContextFlagFixture, ec);
    std::filesystem::remove(badEntryTypeFixture, ec);
    std::filesystem::remove(badDescriptionTypeFixture, ec);
    std::filesystem::remove(unsupportedModeFixture, ec);
    std::filesystem::remove(registerScriptFnFailureFixture, ec);
}

TEST_CASE("Compat fixtures: malformed fixture metadata shape failures are exported as diagnostics artifacts",
          "[compat][fixtures][failure]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    const auto badDependenciesShapeFixture = uniqueTempFixturePath("urpg_bad_dependencies_shape_fixture");
    writeTextFile(badDependenciesShapeFixture,
                  R"({
  "name": "BrokenDependenciesShapeFixture",
  "dependencies": "CorePlugin",
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badDependenciesShapeFixture.string()));
    REQUIRE(pm.getLastError() ==
            ("Fixture plugin 'dependencies' must be array: " + badDependenciesShapeFixture.string()));

    auto diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE_FALSE(diagnostics.empty());
    const auto& dependenciesShapeRow = diagnostics.back();
    REQUIRE(dependenciesShapeRow.value("operation", "") == "load_plugin_dependencies");
    REQUIRE(dependenciesShapeRow.value("plugin", "") == "BrokenDependenciesShapeFixture");

    const auto badDependencyEntryFixture = uniqueTempFixturePath("urpg_bad_dependency_entry_fixture");
    writeTextFile(badDependencyEntryFixture,
                  R"({
  "name": "BrokenDependencyEntryFixture",
  "dependencies": ["CorePlugin", 7],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badDependencyEntryFixture.string()));
    REQUIRE(pm.getLastError() ==
            ("Fixture plugin dependency must be string at index 1: " + badDependencyEntryFixture.string()));

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 2);
    const auto& dependencyEntryRow = diagnostics.back();
    REQUIRE(dependencyEntryRow.value("operation", "") == "load_plugin_dependency_entry");
    REQUIRE(dependencyEntryRow.value("plugin", "") == "BrokenDependencyEntryFixture");

    const auto badParametersShapeFixture = uniqueTempFixturePath("urpg_bad_parameters_shape_fixture");
    writeTextFile(badParametersShapeFixture,
                  R"({
  "name": "BrokenParametersShapeFixture",
  "parameters": ["bad"],
  "commands": [
    {
      "name": "ok",
      "result": 1
    }
  ]
})");

    REQUIRE_FALSE(pm.loadPlugin(badParametersShapeFixture.string()));
    REQUIRE(pm.getLastError() == ("Fixture plugin 'parameters' must be object: " + badParametersShapeFixture.string()));

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 3);
    const auto& parametersShapeRow = diagnostics.back();
    REQUIRE(parametersShapeRow.value("operation", "") == "load_plugin_parameters");
    REQUIRE(parametersShapeRow.value("plugin", "") == "BrokenParametersShapeFixture");

    const auto badCommandsShapeFixture = uniqueTempFixturePath("urpg_bad_commands_shape_fixture");
    writeTextFile(badCommandsShapeFixture,
                  R"({
  "name": "BrokenCommandsShapeFixture",
  "commands": {
    "name": "broken"
  }
})");

    REQUIRE_FALSE(pm.loadPlugin(badCommandsShapeFixture.string()));
    REQUIRE(pm.getLastError() == ("Fixture plugin 'commands' must be array: " + badCommandsShapeFixture.string()));

    diagnostics = parseJsonl(pm.exportFailureDiagnosticsJsonl());
    REQUIRE(diagnostics.size() >= 4);
    const auto& commandsShapeRow = diagnostics.back();
    REQUIRE(commandsShapeRow.value("operation", "") == "load_plugin_commands");
    REQUIRE(commandsShapeRow.value("plugin", "") == "BrokenCommandsShapeFixture");

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(badDependenciesShapeFixture, ec);
    std::filesystem::remove(badDependencyEntryFixture, ec);
    std::filesystem::remove(badParametersShapeFixture, ec);
    std::filesystem::remove(badCommandsShapeFixture, ec);
}
