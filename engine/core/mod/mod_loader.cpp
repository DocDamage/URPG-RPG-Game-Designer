#include "engine/core/mod/mod_loader.h"

#include <algorithm>

namespace urpg::mod {

ModLoader::ModLoader(ModRegistry& registry)
    : m_registry(registry) {}

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

    // Activate the mod
    m_registry.activateMod(manifest.id);

    // Resolve deterministic load order and return it
    try {
        result.loadOrder = m_registry.resolveLoadOrder();
    } catch (const std::exception& e) {
        // Circular dependency detected — roll back
        m_registry.deactivateMod(manifest.id);
        m_registry.unregisterMod(manifest.id);
        m_sandboxPolicies.erase(manifest.id);
        result.success = false;
        result.errorMessage = std::string("Load-order resolution failed: ") + e.what();
        return result;
    }

    result.success = true;
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

} // namespace urpg::mod
