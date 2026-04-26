#include "engine/core/mod/mod_loader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace urpg::mod {

namespace {

std::string readScriptFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        return {};
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string normalizeModScriptExports(std::string source) {
    const auto replaceAll = [](std::string& text, const std::string& needle, const std::string& replacement) {
        size_t offset = 0;
        while ((offset = text.find(needle, offset)) != std::string::npos) {
            text.replace(offset, needle.size(), replacement);
            offset += replacement.size();
        }
    };

    replaceAll(source, "export function activate", "globalThis.activate = function activate");
    replaceAll(source, "export function deactivate", "globalThis.deactivate = function deactivate");
    return source;
}

bool containsToken(const std::string& source, const std::string& token) {
    return source.find(token) != std::string::npos;
}

std::string sandboxViolationForSource(const std::string& source, const ModSandboxPolicy& policy) {
    if (!policy.allowNetworkAccess && (containsToken(source, "fetch(") || containsToken(source, "XMLHttpRequest") ||
                                       containsToken(source, "WebSocket("))) {
        return "Sandbox violation: network access is denied";
    }
    if (!policy.allowFileSystemRead &&
        (containsToken(source, "readFile") || containsToken(source, "File(") || containsToken(source, "require(\"fs") ||
         containsToken(source, "require('fs"))) {
        return "Sandbox violation: file-system read access is denied";
    }
    if (!policy.allowFileSystemWrite && (containsToken(source, "writeFile") || containsToken(source, "appendFile"))) {
        return "Sandbox violation: file-system write access is denied";
    }
    if (!policy.allowNativeInterop && (containsToken(source, "NativeInterop") || containsToken(source, "process.") ||
                                       containsToken(source, "std."))) {
        return "Sandbox violation: native interop is denied";
    }
    return {};
}

std::string makeModContextBootstrapSource() {
    return R"JS(
globalThis.__urpg_mod_commands = {};
globalThis.__urpg_mod_command_names = [];
globalThis.__urpg_mod_command_dispatch = function(commandId, args) {
  var handler = globalThis.__urpg_mod_commands[String(commandId || "")];
  if (typeof handler !== "function") {
    throw new Error("Mod command not found: " + commandId);
  }
  return handler.apply(undefined, Array.isArray(args) ? args : []);
};
globalThis.__urpg_mod_context = {
  commands: {
    register: function(commandId, handler) {
      if (typeof handler !== "function") {
        throw new TypeError("Mod command handler must be a function");
      }
      var key = String(commandId || "");
      if (!key) {
        throw new TypeError("Mod command id cannot be empty");
      }
      globalThis.__urpg_mod_commands[key] = handler;
      if (globalThis.__urpg_mod_command_names.indexOf(key) < 0) {
        globalThis.__urpg_mod_command_names.push(key);
      }
      URPG_modCommandRegistered(key);
      return true;
    }
  },
  diagnostics: {
    info: function(code, message) {
      URPG_modDiagnostic("info", String(code || ""), String(message || ""));
    },
    warn: function(code, message) {
      URPG_modDiagnostic("warning", String(code || ""), String(message || ""));
    },
    error: function(code, message) {
      URPG_modDiagnostic("error", String(code || ""), String(message || ""));
    }
  }
};
)JS";
}

} // namespace

ModLoader::ModLoader(ModRegistry& registry) : m_registry(registry), m_runtimeInitialized(m_runtime.initialize()) {}

std::vector<ModContractFailure> ModLoader::validateContract(const ModManifest& manifest) const {
    std::vector<ModContractFailure> failures;

    if (m_contract.requireId && manifest.id.empty()) {
        failures.push_back({manifest.id, "id", "id field is required but empty"});
    }
    if (m_contract.requireName && manifest.name.empty()) {
        failures.push_back({manifest.id, "name", "name field is required but empty"});
    }
    if (m_contract.requireVersion && manifest.version.empty()) {
        failures.push_back({manifest.id, "version", "version field is required but empty"});
    }
    if (m_contract.requireEntryPoint && manifest.entryPoint.empty()) {
        failures.push_back({manifest.id, "entryPoint", "entryPoint field is required but empty"});
    }

    return failures;
}

ModLoadResult ModLoader::loadMod(const ModManifest& manifest, const ModSandboxPolicy& policy) {
    auto result = registerMod(manifest, policy);
    if (!result.success) {
        return result;
    }

    result = activateMod(manifest.id);
    return result;
}

ModLoadResult ModLoader::registerMod(const ModManifest& manifest, const ModSandboxPolicy& policy) {
    ModLoadResult result;
    result.modId = manifest.id;

    // Contract check
    const auto failures = validateContract(manifest);
    if (!failures.empty()) {
        result.success = false;
        result.errorMessage = "Contract violation: " + failures[0].field + " — " + failures[0].reason;
        return result;
    }

    // Register
    if (!m_registry.registerMod(manifest)) {
        result.success = false;
        result.errorMessage = "Mod already registered: " + manifest.id;
        return result;
    }

    // Store sandbox policy
    m_sandboxPolicies[manifest.id] = policy;
    RuntimeState runtimeState;
    runtimeState.entryPoint = manifest.entryPoint;
    m_runtimeStates[manifest.id] = std::move(runtimeState);

    // Resolve deterministic load order and return it.
    try {
        result.loadOrder = m_registry.resolveLoadOrder();
    } catch (const std::exception& e) {
        // Circular dependency detected — roll back
        m_registry.unregisterMod(manifest.id);
        m_sandboxPolicies.erase(manifest.id);
        m_runtimeStates.erase(manifest.id);
        result.success = false;
        result.errorMessage = std::string("Load-order resolution failed: ") + e.what();
        return result;
    }

    result.success = true;
    return result;
}

ModLoadResult ModLoader::activateMod(const std::string& modId) {
    ModLoadResult result;
    result.modId = modId;

    if (!m_registry.getMod(modId).has_value()) {
        result.success = false;
        result.errorMessage = "Unknown mod: " + modId;
        return result;
    }

    if (!m_registry.activateMod(modId)) {
        result.success = false;
        result.errorMessage = "Failed to activate mod: " + modId;
        return result;
    }

    try {
        result.loadOrder = m_registry.resolveLoadOrder();
    } catch (const std::exception& e) {
        m_registry.deactivateMod(modId);
        result.success = false;
        result.errorMessage = std::string("Load-order resolution failed: ") + e.what();
        return result;
    }

    const auto manifest = m_registry.getMod(modId);
    const auto policy = getSandboxPolicy(modId).value_or(ModSandboxPolicy{});
    RuntimeState& runtimeState = m_runtimeStates[modId];
    runtimeState.entryPoint = manifest->entryPoint;
    runtimeState.commandIds.clear();
    runtimeState.diagnostics.clear();
    runtimeState.scriptLoaded = false;
    runtimeState.active = false;
    const auto resetRuntimeState = [&]() {
        runtimeState.entryPoint = manifest->entryPoint;
        runtimeState.commandIds.clear();
        runtimeState.diagnostics.clear();
        runtimeState.scriptLoaded = false;
        runtimeState.active = false;
    };

    const std::filesystem::path entryPath = manifest->entryPoint;
    const bool hasExecutableEntry =
        !manifest->entryPoint.empty() &&
        (entryPath.is_absolute() ? std::filesystem::exists(entryPath)
                                 : std::filesystem::exists(entryPath) ||
                                       std::filesystem::exists(std::filesystem::current_path() / entryPath));

    if (hasExecutableEntry) {
        if (!m_runtimeInitialized) {
            m_registry.deactivateMod(modId);
            result.success = false;
            result.errorMessage = "QuickJS mod runtime is unavailable";
            return result;
        }

        const std::filesystem::path resolvedEntry = entryPath.is_absolute() || std::filesystem::exists(entryPath)
                                                        ? entryPath
                                                        : std::filesystem::current_path() / entryPath;
        const std::string source = readScriptFile(resolvedEntry);
        if (source.empty()) {
            m_registry.deactivateMod(modId);
            resetRuntimeState();
            result.success = false;
            result.errorMessage = "Failed to read mod entrypoint: " + resolvedEntry.string();
            return result;
        }

        if (const std::string violation = sandboxViolationForSource(source, policy); !violation.empty()) {
            m_registry.deactivateMod(modId);
            resetRuntimeState();
            result.success = false;
            result.errorMessage = violation + " for mod: " + modId;
            return result;
        }

        if (const auto previousContext = m_runtime.getContextId(modId)) {
            m_runtime.destroyContext(*previousContext);
        }
        const auto contextId = m_runtime.createContext(modId, compat::QuickJSConfig{});
        if (!contextId) {
            m_registry.deactivateMod(modId);
            resetRuntimeState();
            result.success = false;
            result.errorMessage = "Failed to create QuickJS context for mod: " + modId;
            return result;
        }

        compat::QuickJSContext* context = m_runtime.getContext(*contextId);
        if (!context ||
            !context->registerFunction("URPG_modCommandRegistered",
                                       [this, modId](const std::vector<urpg::Value>& args) -> urpg::Value {
                                           if (!args.empty()) {
                                               if (const auto* commandId = std::get_if<std::string>(&args.front().v)) {
                                                   auto& commands = m_runtimeStates[modId].commandIds;
                                                   if (std::find(commands.begin(), commands.end(), *commandId) ==
                                                       commands.end()) {
                                                       commands.push_back(*commandId);
                                                   }
                                               }
                                           }
                                           return urpg::Value::Nil();
                                       }) ||
            !context->registerFunction(
                "URPG_modDiagnostic", [this, modId](const std::vector<urpg::Value>& args) -> urpg::Value {
                    ModDiagnostic diagnostic;
                    diagnostic.severity = args.size() > 0 && std::holds_alternative<std::string>(args[0].v)
                                              ? std::get<std::string>(args[0].v)
                                              : "info";
                    diagnostic.code = args.size() > 1 && std::holds_alternative<std::string>(args[1].v)
                                          ? std::get<std::string>(args[1].v)
                                          : "";
                    diagnostic.message = args.size() > 2 && std::holds_alternative<std::string>(args[2].v)
                                             ? std::get<std::string>(args[2].v)
                                             : "";
                    m_runtimeStates[modId].diagnostics.push_back(std::move(diagnostic));
                    return urpg::Value::Nil();
                })) {
            m_runtime.destroyContext(*contextId);
            m_registry.deactivateMod(modId);
            resetRuntimeState();
            result.success = false;
            result.errorMessage = "Failed to register mod host bindings: " + modId;
            return result;
        }

        auto rollbackRuntimeActivation = [&]() {
            if (const auto activeContext = m_runtime.getContextId(modId)) {
                m_runtime.destroyContext(*activeContext);
            }
            m_registry.deactivateMod(modId);
            resetRuntimeState();
        };

        const auto bootstrapResult = context->eval(makeModContextBootstrapSource(), modId + "#bootstrap");
        if (!bootstrapResult.success) {
            rollbackRuntimeActivation();
            result.success = false;
            result.errorMessage = "Mod runtime bootstrap failed: " + bootstrapResult.error;
            return result;
        }

        const auto evalResult = context->eval(normalizeModScriptExports(source), resolvedEntry.string());
        if (!evalResult.success) {
            rollbackRuntimeActivation();
            result.success = false;
            result.errorMessage = "Mod script evaluation failed: " + evalResult.error;
            return result;
        }

        const auto activateResult = context->eval("activate(globalThis.__urpg_mod_context);", modId + "#activate");
        if (!activateResult.success) {
            rollbackRuntimeActivation();
            result.success = false;
            result.errorMessage = "Mod activate hook failed: " + activateResult.error;
            return result;
        }

        runtimeState.scriptLoaded = true;
    }

    runtimeState.active = true;

    result.success = true;
    return result;
}

ModLoadResult ModLoader::deactivateMod(const std::string& modId) {
    ModLoadResult result;
    result.modId = modId;

    if (!m_registry.getMod(modId).has_value()) {
        result.success = false;
        result.errorMessage = "Unknown mod: " + modId;
        return result;
    }

    if (!m_registry.deactivateMod(modId)) {
        result.success = false;
        result.errorMessage = "Failed to deactivate mod: " + modId;
        return result;
    }

    if (const auto contextId = m_runtime.getContextId(modId)) {
        if (compat::QuickJSContext* context = m_runtime.getContext(*contextId)) {
            const auto maybeDeactivate = context->getGlobal("deactivate");
            if (maybeDeactivate.has_value()) {
                const auto deactivateResult =
                    context->eval("deactivate(globalThis.__urpg_mod_context);", modId + "#deactivate");
                if (!deactivateResult.success) {
                    m_registry.activateMod(modId);
                    result.success = false;
                    result.errorMessage = "Mod deactivate hook failed: " + deactivateResult.error;
                    return result;
                }
            }
        }
        m_runtime.destroyContext(*contextId);
    }
    if (auto stateIt = m_runtimeStates.find(modId); stateIt != m_runtimeStates.end()) {
        stateIt->second.active = false;
        stateIt->second.scriptLoaded = false;
        stateIt->second.commandIds.clear();
    }

    try {
        result.loadOrder = m_registry.resolveLoadOrder();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("Load-order resolution failed: ") + e.what();
        return result;
    }

    result.success = true;
    return result;
}

ModLoadResult ModLoader::reloadMod(const std::string& modId) {
    ModLoadResult result;
    result.modId = modId;

    const auto manifest = m_registry.getMod(modId);
    if (!manifest.has_value()) {
        result.success = false;
        result.errorMessage = "Unknown mod: " + modId;
        return result;
    }

    const auto policy = getSandboxPolicy(modId).value_or(ModSandboxPolicy{});
    const auto unloadResult = unloadMod(modId);
    if (!unloadResult.success) {
        return unloadResult;
    }

    result = loadMod(*manifest, policy);
    if (!result.success && !m_registry.getMod(manifest->id).has_value()) {
        (void)registerMod(*manifest, policy);
    }
    return result;
}

ModLoadResult ModLoader::unloadMod(const std::string& modId) {
    ModLoadResult result;
    result.modId = modId;

    if (!m_registry.getMod(modId).has_value()) {
        result.success = false;
        result.errorMessage = "Unknown mod: " + modId;
        return result;
    }

    // Deactivate dependents first (deterministic: reverse-topological order).
    // resolveLoadOrder gives dependencies-first ordering; dependents of modId
    // appear after modId in that list.  Deactivate them in reverse order.
    std::vector<std::string> order;
    try {
        order = m_registry.resolveLoadOrder();
    } catch (...) {
        // If order resolution fails, do a best-effort direct deactivation.
        order = {modId};
    }

    // Find position of the target mod
    auto targetIt = std::find(order.begin(), order.end(), modId);
    if (targetIt != order.end()) {
        // All mods after the target in the load order are its dependents.
        // Deactivate in reverse order.
        for (auto it = order.end() - 1; it != targetIt; --it) {
            m_registry.deactivateMod(*it);
        }
    }

    m_registry.deactivateMod(modId);
    m_registry.unregisterMod(modId);
    m_sandboxPolicies.erase(modId);
    if (const auto contextId = m_runtime.getContextId(modId)) {
        m_runtime.destroyContext(*contextId);
    }
    m_runtimeStates.erase(modId);

    // Recompute load order for remaining active mods
    try {
        result.loadOrder = m_registry.resolveLoadOrder();
    } catch (...) {
        result.loadOrder = {};
    }

    result.success = true;
    return result;
}

std::optional<ModSandboxPolicy> ModLoader::getSandboxPolicy(const std::string& modId) const {
    auto it = m_sandboxPolicies.find(modId);
    if (it == m_sandboxPolicies.end()) {
        return std::nullopt;
    }
    return it->second;
}

void ModLoader::setStoreContract(const ModStoreContract& contract) {
    m_contract = contract;
}

std::optional<urpg::Value> ModLoader::executeCommand(const std::string& modId, const std::string& commandId,
                                                     const std::vector<urpg::Value>& args) {
    const auto stateIt = m_runtimeStates.find(modId);
    if (stateIt == m_runtimeStates.end() || !stateIt->second.active) {
        return std::nullopt;
    }

    const auto contextId = m_runtime.getContextId(modId);
    if (!contextId) {
        return std::nullopt;
    }
    compat::QuickJSContext* context = m_runtime.getContext(*contextId);
    if (!context) {
        return std::nullopt;
    }

    std::vector<urpg::Value> callArgs;
    callArgs.reserve(args.size() + 1);
    urpg::Value commandValue;
    commandValue.v = commandId;
    callArgs.push_back(std::move(commandValue));
    callArgs.push_back(urpg::Value::Arr(urpg::Array(args.begin(), args.end())));

    const auto result = context->call("__urpg_mod_command_dispatch", callArgs);
    if (!result.success) {
        m_runtimeStates[modId].diagnostics.push_back({
            "mod.command.failed",
            "error",
            result.error,
        });
        return std::nullopt;
    }
    return result.value;
}

ModRuntimeSnapshot ModLoader::getRuntimeSnapshot(const std::string& modId) const {
    ModRuntimeSnapshot snapshot;
    const auto it = m_runtimeStates.find(modId);
    if (it == m_runtimeStates.end()) {
        return snapshot;
    }
    snapshot.commandIds = it->second.commandIds;
    snapshot.diagnostics = it->second.diagnostics;
    snapshot.scriptLoaded = it->second.scriptLoaded;
    snapshot.active = it->second.active;
    return snapshot;
}

} // namespace urpg::mod
