#include "engine/core/mod/mod_store.h"

#include <fstream>

namespace urpg::mod {

namespace {

void addDiagnostic(std::vector<std::string>* diagnostics, const std::string& message) {
    if (diagnostics != nullptr) {
        diagnostics->push_back(message);
    }
}

ModSandboxPolicy parsePolicy(const nlohmann::json& json) {
    ModSandboxPolicy policy;
    if (!json.is_object()) {
        return policy;
    }
    policy.allowFileSystemRead = json.value("fileSystemRead", false);
    policy.allowFileSystemWrite = json.value("fileSystemWrite", false);
    policy.allowNetworkAccess = json.value("networkAccess", false);
    policy.allowNativeInterop = json.value("nativeInterop", false);
    return policy;
}

ModManifest parseManifest(const nlohmann::json& json) {
    ModManifest manifest;
    manifest.id = json.value("id", "");
    manifest.name = json.value("name", "");
    manifest.version = json.value("version", "");
    manifest.author = json.value("author", "");
    manifest.entryPoint = json.value("entryPoint", "");
    if (const auto it = json.find("dependencies"); it != json.end() && it->is_array()) {
        for (const auto& dependency : *it) {
            if (dependency.is_string()) {
                manifest.dependencies.push_back(dependency.get<std::string>());
            }
        }
    }
    return manifest;
}

nlohmann::json manifestToJson(const ModManifest& manifest) {
    return {
        {"id", manifest.id},
        {"name", manifest.name},
        {"version", manifest.version},
        {"author", manifest.author},
        {"dependencies", manifest.dependencies},
        {"entryPoint", manifest.entryPoint},
    };
}

nlohmann::json policyToJson(const ModSandboxPolicy& policy) {
    return {
        {"fileSystemRead", policy.allowFileSystemRead},
        {"fileSystemWrite", policy.allowFileSystemWrite},
        {"networkAccess", policy.allowNetworkAccess},
        {"nativeInterop", policy.allowNativeInterop},
    };
}

std::filesystem::path resolveEntrypoint(const ModStoreEntry& entry, const std::filesystem::path& catalogRoot) {
    std::filesystem::path entryPoint = entry.manifest.entryPoint;
    if (entryPoint.empty() || entryPoint.is_absolute()) {
        return entryPoint;
    }
    std::filesystem::path root = entry.packageRoot;
    if (!root.is_absolute() && !catalogRoot.empty()) {
        root = catalogRoot / root;
    }
    return root.empty() ? entryPoint : root / entryPoint;
}

} // namespace

bool ModStoreCatalog::loadFromJson(const nlohmann::json& document, std::vector<std::string>* diagnostics) {
    m_entries.clear();
    if (!document.is_object()) {
        addDiagnostic(diagnostics, "mod_store_catalog_not_object");
        return false;
    }
    if (!document.contains("entries") || !document["entries"].is_array()) {
        addDiagnostic(diagnostics, "mod_store_entries_missing");
        return false;
    }

    bool ok = true;
    for (const auto& entryJson : document["entries"]) {
        if (!entryJson.is_object()) {
            addDiagnostic(diagnostics, "mod_store_entry_not_object");
            ok = false;
            continue;
        }
        if (!entryJson.contains("manifest") || !entryJson["manifest"].is_object()) {
            addDiagnostic(diagnostics, "mod_store_entry_manifest_missing");
            ok = false;
            continue;
        }

        ModStoreEntry entry;
        entry.id = entryJson.value("id", "");
        entry.displayName = entryJson.value("displayName", "");
        entry.version = entryJson.value("version", "");
        entry.publisher = entryJson.value("publisher", "");
        entry.packageRoot = entryJson.value("packageRoot", "");
        entry.verified = entryJson.value("verified", false);
        entry.manifest = parseManifest(entryJson["manifest"]);
        entry.sandboxPolicy = parsePolicy(entryJson.value("sandboxPolicy", nlohmann::json::object()));
        if (const auto tagsIt = entryJson.find("tags"); tagsIt != entryJson.end() && tagsIt->is_array()) {
            for (const auto& tag : *tagsIt) {
                if (tag.is_string()) {
                    entry.tags.push_back(tag.get<std::string>());
                }
            }
        }

        if (entry.id.empty() || entry.manifest.id.empty()) {
            addDiagnostic(diagnostics, "mod_store_entry_missing_id");
            ok = false;
            continue;
        }
        m_entries.push_back(std::move(entry));
    }

    return ok;
}

bool ModStoreCatalog::loadFromFile(const std::filesystem::path& path, std::vector<std::string>* diagnostics) {
    std::ifstream input(path);
    if (!input) {
        addDiagnostic(diagnostics, "mod_store_catalog_open_failed");
        return false;
    }
    nlohmann::json document;
    input >> document;
    return loadFromJson(document, diagnostics);
}

std::optional<ModStoreEntry> ModStoreCatalog::findEntry(const std::string& entryId) const {
    for (const auto& entry : m_entries) {
        if (entry.id == entryId) {
            return entry;
        }
    }
    return std::nullopt;
}

nlohmann::json ModStoreCatalog::toJson() const {
    nlohmann::json entries = nlohmann::json::array();
    for (const auto& entry : m_entries) {
        entries.push_back({
            {"id", entry.id},
            {"displayName", entry.displayName},
            {"version", entry.version},
            {"publisher", entry.publisher},
            {"packageRoot", entry.packageRoot},
            {"verified", entry.verified},
            {"tags", entry.tags},
            {"manifest", manifestToJson(entry.manifest)},
            {"sandboxPolicy", policyToJson(entry.sandboxPolicy)},
        });
    }
    return {{"version", 1}, {"entries", entries}};
}

ModStoreInstaller::ModStoreInstaller(ModLoader& loader) : m_loader(loader) {}

ModStoreInstallResult ModStoreInstaller::install(const ModStoreEntry& entry, const std::filesystem::path& catalogRoot) {
    ModStoreInstallResult result;
    result.entryId = entry.id;
    result.modId = entry.manifest.id;

    ModManifest manifest = entry.manifest;
    manifest.entryPoint = resolveEntrypoint(entry, catalogRoot).generic_string();
    const auto loadResult = m_loader.registerMod(manifest, entry.sandboxPolicy);
    result.success = loadResult.success;
    result.errorMessage = loadResult.errorMessage;
    result.loadOrder = loadResult.loadOrder;
    return result;
}

} // namespace urpg::mod
