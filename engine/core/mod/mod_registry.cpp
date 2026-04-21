#include "engine/core/mod/mod_registry.h"

#include <queue>
#include <stdexcept>
#include <unordered_set>

namespace urpg::mod {

bool ModRegistry::registerMod(const ModManifest& manifest) {
    if (mods_.find(manifest.id) != mods_.end()) {
        return false;
    }

    ModInstance instance;
    instance.manifest = manifest;
    instance.active = false;
    instance.loadOrder = 0;

    mods_[manifest.id] = std::move(instance);
    return true;
}

bool ModRegistry::unregisterMod(const std::string& id) {
    auto it = mods_.find(id);
    if (it == mods_.end()) {
        return false;
    }

    mods_.erase(it);
    return true;
}

std::optional<ModManifest> ModRegistry::getMod(const std::string& id) const {
    auto it = mods_.find(id);
    if (it == mods_.end()) {
        return std::nullopt;
    }

    return it->second.manifest;
}

std::vector<std::string> ModRegistry::resolveLoadOrder() const {
    std::unordered_map<std::string, int32_t> inDegree;
    std::unordered_map<std::string, std::vector<std::string>> adjacency;

    for (const auto& [id, instance] : mods_) {
        inDegree[id] = 0;
    }

    for (const auto& [id, instance] : mods_) {
        for (const auto& dep : instance.manifest.dependencies) {
            if (mods_.find(dep) != mods_.end()) {
                adjacency[dep].push_back(id);
                ++inDegree[id];
            }
        }
    }

    std::queue<std::string> queue;
    for (const auto& [id, degree] : inDegree) {
        if (degree == 0) {
            queue.push(id);
        }
    }

    std::vector<std::string> result;
    result.reserve(mods_.size());

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        result.push_back(current);

        auto it = adjacency.find(current);
        if (it != adjacency.end()) {
            for (const auto& neighbor : it->second) {
                --inDegree[neighbor];
                if (inDegree[neighbor] == 0) {
                    queue.push(neighbor);
                }
            }
        }
    }

    if (result.size() != mods_.size()) {
        throw std::runtime_error("ModRegistry: circular dependency detected");
    }

    return result;
}

bool ModRegistry::activateMod(const std::string& id) {
    auto it = mods_.find(id);
    if (it == mods_.end()) {
        return false;
    }

    it->second.active = true;
    return true;
}

bool ModRegistry::deactivateMod(const std::string& id) {
    auto it = mods_.find(id);
    if (it == mods_.end()) {
        return false;
    }

    it->second.active = false;
    return true;
}

std::vector<std::string> ModRegistry::listActiveMods() const {
    std::vector<std::string> result;
    for (const auto& [id, instance] : mods_) {
        if (instance.active) {
            result.push_back(id);
        }
    }
    return result;
}

nlohmann::json ModRegistry::saveState() const {
    nlohmann::json state;
    state["version"] = "1.0.0";
    state["mods"] = nlohmann::json::array();

    for (const auto& [id, instance] : mods_) {
        nlohmann::json modJson;
        modJson["id"] = id;
        modJson["active"] = instance.active;
        modJson["loadOrder"] = instance.loadOrder;
        state["mods"].push_back(modJson);
    }

    return state;
}

void ModRegistry::loadState(const nlohmann::json& state) {
    mods_.clear();

    if (!state.contains("mods") || !state["mods"].is_array()) {
        return;
    }

    for (const auto& modJson : state["mods"]) {
        if (!modJson.contains("id") || !modJson["id"].is_string()) {
            continue;
        }

        std::string id = modJson["id"];
        ModManifest manifest;
        manifest.id = id;
        manifest.name = id;
        manifest.version = "unknown";

        ModInstance instance;
        instance.manifest = std::move(manifest);
        instance.active = modJson.value("active", false);
        instance.loadOrder = modJson.value("loadOrder", 0);

        mods_[id] = std::move(instance);
    }
}

} // namespace urpg::mod
