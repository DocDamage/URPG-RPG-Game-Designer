#pragma once

#include "engine/core/mod/mod_hot_loader.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/mod/mod_store.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace urpg::editor {

class ModManagerPanel {
  public:
    ModManagerPanel() = default;

    void bindRegistry(urpg::mod::ModRegistry* registry);
    void bindLoader(urpg::mod::ModLoader* loader);
    void bindHotLoader(urpg::mod::ModHotLoader* hot_loader);
    void bindStoreCatalog(urpg::mod::ModStoreCatalog* store_catalog);
    void bindMarketplaceProviderProfile(const urpg::mod::ModMarketplaceProviderProfile* provider_profile);
    void render();

    bool importManifest(const std::filesystem::path& manifest_path, const urpg::mod::ModSandboxPolicy& policy = {});
    bool registerManifest(const urpg::mod::ModManifest& manifest, const urpg::mod::ModSandboxPolicy& policy = {});
    bool activateMod(const std::string& id);
    bool deactivateMod(const std::string& id);
    bool reloadMod(const std::string& id);
    bool trackHotLoadMod(const std::string& id);
    size_t trackRegisteredHotLoadMods();
    urpg::mod::ModHotLoadPollResult pollHotLoader();
    void clearHotLoadEventLog();
    bool installStoreEntry(const std::string& entry_id, const std::filesystem::path& catalog_root = {});
    void clearLastAction();

    nlohmann::json lastRenderSnapshot() const;

  private:
    static urpg::mod::ModManifest parseManifestJson(const nlohmann::json& manifest_json);
    void recordLoadResult(const std::string& action, const urpg::mod::ModLoadResult& result);

    urpg::mod::ModRegistry* registry_ = nullptr;
    urpg::mod::ModLoader* loader_ = nullptr;
    urpg::mod::ModHotLoader* hot_loader_ = nullptr;
    urpg::mod::ModStoreCatalog* store_catalog_ = nullptr;
    const urpg::mod::ModMarketplaceProviderProfile* marketplace_provider_profile_ = nullptr;
    nlohmann::json last_render_snapshot_;
    nlohmann::json last_action_snapshot_;
    urpg::mod::ModHotLoadPollResult last_hot_poll_result_;
};

} // namespace urpg::editor
