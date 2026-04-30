#pragma once

#include <algorithm>
#include <cctype>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <set>
#include <vector>

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
            std::set<std::string> seenDependencies;
            for (const auto& dep : j["dependencies"]) {
                PluginDependency dependency;
                if (dep.is_string()) {
                    dependency = {dep.get<std::string>(), "*", false};
                } else if (dep.is_object()) {
                    dependency = {dep.value("id", ""), dep.value("version", "*"), dep.value("optional", false)};
                } else {
                    continue;
                }
                if (isBlank(dependency.pluginId) || !seenDependencies.insert(dependency.pluginId).second) {
                    continue;
                }
                m.dependencies.push_back(std::move(dependency));
            }
            std::sort(m.dependencies.begin(), m.dependencies.end(), [](const auto& left, const auto& right) {
                return left.pluginId < right.pluginId;
            });
        }

        if (j.contains("permissions") && j["permissions"].is_array()) {
            for (const auto& permission : j["permissions"]) {
                if (permission.is_string() && !isBlank(permission.get<std::string>())) {
                    m.permissions.push_back(permission.get<std::string>());
                }
            }
            std::sort(m.permissions.begin(), m.permissions.end());
            m.permissions.erase(std::unique(m.permissions.begin(), m.permissions.end()), m.permissions.end());
        }

        if (j.contains("parameters") && j["parameters"].is_object()) {
            for (auto& [key, value] : j["parameters"].items()) {
                m.parameters[key] = value.dump();
            }
        }

        return m;
    }

  private:
    static bool isBlank(const std::string& value) {
        return std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
    }
};

} // namespace urpg::plugin
