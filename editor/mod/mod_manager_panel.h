#pragma once

#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace urpg::editor {

class ModManagerPanel {
  public:
    ModManagerPanel() = default;

    void bindRegistry(urpg::mod::ModRegistry* registry);
    void bindLoader(urpg::mod::ModLoader* loader);
    void render();

    bool importManifest(const std::filesystem::path& manifest_path, const urpg::mod::ModSandboxPolicy& policy = {});
    bool registerManifest(const urpg::mod::ModManifest& manifest, const urpg::mod::ModSandboxPolicy& policy = {});
    bool activateMod(const std::string& id);
    bool deactivateMod(const std::string& id);
    bool reloadMod(const std::string& id);
    void clearLastAction();

    nlohmann::json lastRenderSnapshot() const;

  private:
    static urpg::mod::ModManifest parseManifestJson(const nlohmann::json& manifest_json);
    void recordLoadResult(const std::string& action, const urpg::mod::ModLoadResult& result);

    urpg::mod::ModRegistry* registry_ = nullptr;
    urpg::mod::ModLoader* loader_ = nullptr;
    nlohmann::json last_render_snapshot_;
    nlohmann::json last_action_snapshot_;
};

} // namespace urpg::editor
