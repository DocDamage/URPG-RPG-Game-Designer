#pragma once

#include "engine/core/mod/mod_loader.h"
#include "engine/core/release/provider_profile_status.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace urpg::mod {

struct ModStoreEntry {
    std::string id;
    std::string displayName;
    std::string version;
    std::string publisher;
    std::string packageRoot;
    ModManifest manifest;
    ModSandboxPolicy sandboxPolicy;
    std::vector<std::string> tags;
    bool verified = false;
};

struct ModStoreInstallResult {
    bool success = false;
    std::string entryId;
    std::string modId;
    std::string errorMessage;
    std::vector<std::string> loadOrder;
};

struct ModMarketplaceProviderProfile {
    std::string profileId;
    std::string providerId = "disabled";
    std::string endpoint;
    std::string commandExecutable;
    std::string credentialSourceCategory = "none";
    bool reviewed = false;
    std::string reviewedBy;
    std::string reviewedAt;
    std::string lastTestResult = "not_run";

    nlohmann::json toJson() const;
    static ModMarketplaceProviderProfile fromJson(const nlohmann::json& json);
};

urpg::release::ProviderProfileStatus modMarketplaceProviderProfileStatus(
    const ModMarketplaceProviderProfile& profile);

class ModStoreCatalog {
public:
    bool loadFromJson(const nlohmann::json& document, std::vector<std::string>* diagnostics = nullptr);
    bool loadFromFile(const std::filesystem::path& path, std::vector<std::string>* diagnostics = nullptr);

    const std::vector<ModStoreEntry>& entries() const { return m_entries; }
    std::optional<ModStoreEntry> findEntry(const std::string& entryId) const;
    nlohmann::json toJson() const;

private:
    std::vector<ModStoreEntry> m_entries;
};

class ModStoreInstaller {
public:
    explicit ModStoreInstaller(ModLoader& loader);

    ModStoreInstallResult install(const ModStoreEntry& entry, const std::filesystem::path& catalogRoot = {});

private:
    ModLoader& m_loader;
};

} // namespace urpg::mod
