#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace urpg::plugin {

/**
 * @brief Dependency definition for a plugin.
 */
struct PluginDependency {
    std::string pluginId;
    std::string versionRange; // SemVer range (e.g. "^1.0.0")
    bool isOptional = false;
};

/**
 * @brief Manifest data for a URPG plugin.
 * Maps to plugin.json schema.
 */
struct PluginManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    
    std::vector<PluginDependency> dependencies;
    std::vector<std::string> permissions;
    std::map<std::string, std::string> parameters; // Default parameter values

    static PluginManifest fromJson(const nlohmann::json& j) {
        PluginManifest m;
        m.id = j.value("id", "");
        m.name = j.value("name", "");
        m.version = j.value("version", "0.1.0");
        m.author = j.value("author", "Unknown");
        m.description = j.value("description", "");

        if (j.contains("dependencies") && j["dependencies"].is_array()) {
            for (const auto& dep : j["dependencies"]) {
                if (dep.is_string()) {
                    m.dependencies.push_back({dep.get<std::string>(), "*", false});
                } else if (dep.is_object()) {
                    m.dependencies.push_back({
                        dep.value("id", ""),
                        dep.value("version", "*"),
                        dep.value("optional", false)
                    });
                }
            }
        }

        if (j.contains("permissions") && j["permissions"].is_array()) {
            m.permissions = j["permissions"].get<std::vector<std::string>>();
        }

        if (j.contains("parameters") && j["parameters"].is_object()) {
            for (auto& [key, value] : j["parameters"].items()) {
                m.parameters[key] = value.dump();
            }
        }

        return m;
    }
};

} // namespace urpg::plugin
