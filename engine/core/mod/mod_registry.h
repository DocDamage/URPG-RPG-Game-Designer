#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::mod {

struct ModManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::vector<std::string> dependencies;
    std::string entryPoint;
};

struct ModInstance {
    ModManifest manifest;
    bool active = false;
    int32_t loadOrder = 0;
};

class ModRegistry {
public:
    ModRegistry() = default;

    bool registerMod(const ModManifest& manifest);
    bool unregisterMod(const std::string& id);

    std::optional<ModManifest> getMod(const std::string& id) const;
    std::vector<ModManifest> listMods() const;
    std::vector<std::string> resolveLoadOrder() const;

    bool activateMod(const std::string& id);
    bool deactivateMod(const std::string& id);
    std::vector<std::string> listActiveMods() const;

    nlohmann::json saveState() const;
    void loadState(const nlohmann::json& state);

private:
    std::unordered_map<std::string, ModInstance> mods_;
};

} // namespace urpg::mod
