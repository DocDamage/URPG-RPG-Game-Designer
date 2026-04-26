#pragma once

#include "engine/core/mod/mod_registry.h"
#include "runtimes/compat_js/quickjs_runtime.h"

#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::mod {

/**
 * @brief Result of a mod load or unload operation.
 */
struct ModLoadResult {
    bool success = false;
    std::string modId;
    std::string errorMessage;
    /** Ordered list of mods that were (re-)activated after this operation. */
    std::vector<std::string> loadOrder;
};

/**
 * @brief Permissions granted to a sandboxed mod.
 *
 * All flags default to false (deny-by-default sandbox).
 */
struct ModSandboxPolicy {
    bool allowFileSystemRead = false;
    bool allowFileSystemWrite = false;
    bool allowNetworkAccess = false;
    bool allowNativeInterop = false;
};

/**
 * @brief Contract describing a mod-store entry.
 *
 * Used by ModLoader to validate that an installed mod manifest satisfies the
 * minimum requirements expected by the store and the engine before the mod is
 * allowed to load.
 */
struct ModStoreContract {
    /** Minimum required manifest fields. */
    bool requireId = true;
    bool requireName = true;
    bool requireVersion = true;
    bool requireEntryPoint = true;
    /** Whether unknown/unsupported manifest fields are treated as errors. */
    bool rejectUnknownFields = false;
};

/**
 * @brief Failure report produced when a mod-store contract check fails.
 */
struct ModContractFailure {
    std::string modId;
    std::string field;
    std::string reason;
};

struct ModDiagnostic {
    std::string code;
    std::string severity;
    std::string message;
};

struct ModRuntimeSnapshot {
    std::vector<std::string> commandIds;
    std::vector<ModDiagnostic> diagnostics;
    bool scriptLoaded = false;
    bool active = false;
};

/**
 * @brief Provides live mod loading, deterministic unload, sandboxing, and
 *        mod-store contract enforcement on top of a ModRegistry.
 *
 * ModLoader owns runtime state for sandboxed QuickJS mod contexts in addition
 * to registry lifecycle, load-order computation, sandbox policy attachment,
 * and store-contract validation.
 */
class ModLoader {
  public:
    explicit ModLoader(ModRegistry& registry);

    /**
     * @brief Validate @p manifest against the current store contract and, if
     *        it passes, register it with the registry, resolve load order, and
     *        record the sandbox policy for subsequent activation calls.
     *
     * @param manifest  Manifest describing the mod.
     * @param policy    Sandbox permissions to apply when the mod is activated.
     * @return          ModLoadResult with success flag and resolved load order.
     */
    ModLoadResult loadMod(const ModManifest& manifest, const ModSandboxPolicy& policy = {});

    /**
     * @brief Validate and register @p manifest without activating it.
     *
     * This supports editor import workflows where a manifest is staged first
     * and activated explicitly by the user after reviewing validation and
     * sandbox policy details.
     */
    ModLoadResult registerMod(const ModManifest& manifest, const ModSandboxPolicy& policy = {});

    /**
     * @brief Activate an already registered mod and return resolved load order.
     */
    ModLoadResult activateMod(const std::string& modId);

    /**
     * @brief Deactivate an already registered mod and return resolved load order.
     */
    ModLoadResult deactivateMod(const std::string& modId);

    /**
     * @brief Re-register and activate an existing mod using its stored manifest
     *        and sandbox policy.
     */
    ModLoadResult reloadMod(const std::string& modId);

    /**
     * @brief Deactivate and unregister a mod, then recompute load order for
     *        remaining active mods.
     *
     * The unload is deterministic: dependents of the named mod are deactivated
     * first in reverse load-order before the mod itself is removed.
     *
     * @param modId  Identifier of the mod to unload.
     * @return       ModLoadResult describing the new active load order.
     */
    ModLoadResult unloadMod(const std::string& modId);

    /**
     * @brief Return the sandbox policy stored for @p modId, if any.
     */
    std::optional<ModSandboxPolicy> getSandboxPolicy(const std::string& modId) const;

    /**
     * @brief Configure the contract that every loaded mod must satisfy.
     */
    void setStoreContract(const ModStoreContract& contract);

    /**
     * @brief Validate @p manifest against the current store contract without
     *        registering it.
     *
     * @return Empty vector on success; one entry per contract violation.
     */
    std::vector<ModContractFailure> validateContract(const ModManifest& manifest) const;

    /**
     * @brief Execute a command registered by an active mod script.
     */
    std::optional<urpg::Value> executeCommand(const std::string& modId, const std::string& commandId,
                                              const std::vector<urpg::Value>& args = {});

    /**
     * @brief Return runtime commands/diagnostics observed for a mod.
     */
    ModRuntimeSnapshot getRuntimeSnapshot(const std::string& modId) const;

  private:
    struct RuntimeState {
        std::string entryPoint;
        std::vector<std::string> commandIds;
        std::vector<ModDiagnostic> diagnostics;
        bool scriptLoaded = false;
        bool active = false;
    };

    ModRegistry& m_registry;
    ModStoreContract m_contract;
    std::unordered_map<std::string, ModSandboxPolicy> m_sandboxPolicies;
    compat::QuickJSRuntime m_runtime;
    bool m_runtimeInitialized = false;
    std::unordered_map<std::string, RuntimeState> m_runtimeStates;
};

} // namespace urpg::mod
