#include "plugin_host.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace urpg::editor {

namespace fs = std::filesystem;

void PluginHost::discoverPlugins(const std::string& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        logError("Plugin directory does not exist: " + directory);
        return;
    }

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_directory()) {
                // Placeholder for discovery logic: manifest.json vs .dll/.so/.js
                // Wave 7.1: We identify potential plugins for later loading.
                PluginInfo info;
                info.id = entry.path().filename().string();
                info.name = "Discovered: " + info.id;
                info.status = PluginStatus::Unloaded;
                
                // Avoid duplicates
                auto it = std::find_if(m_availablePlugins.begin(), m_availablePlugins.end(),
                    [&info](const PluginInfo& p) { return p.id == info.id; });
                
                if (it == m_availablePlugins.end()) {
                    m_availablePlugins.push_back(info);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        logError("Error scanning plugin directory: " + std::string(e.what()));
    }
}

bool PluginHost::loadPlugin(const std::string& pluginId) {
    // Wave 7.1: Skeleton loading for native and scripted hooks.
    auto it = std::find_if(m_availablePlugins.begin(), m_availablePlugins.end(),
        [&pluginId](const PluginInfo& p) { return p.id == pluginId; });

    if (it == m_availablePlugins.end()) {
        logError("Cannot load unknown plugin: " + pluginId);
        return false;
    }

    if (it->status == PluginStatus::Active) {
        return true; // Already active
    }

    it->status = PluginStatus::Loading;
    
    // Wave 7.2/7.3 will handle the actual DLL/JS loading here.
    // For now, we simulate success for registered native instances.
    if (m_activePlugins.count(pluginId)) {
        m_activePlugins[pluginId]->onInitialize();
        it->status = PluginStatus::Active;
        return true;
    }

    it->status = PluginStatus::Error;
    logError("No executable component found for plugin: " + pluginId);
    return false;
}

void PluginHost::unloadPlugin(const std::string& pluginId) {
    auto it = std::find_if(m_availablePlugins.begin(), m_availablePlugins.end(),
        [&pluginId](const PluginInfo& p) { return p.id == pluginId; });

    if (it != m_availablePlugins.end()) {
        it->status = PluginStatus::Unloaded;
    }

    if (m_activePlugins.count(pluginId)) {
        m_activePlugins[pluginId]->onShutdown();
        // For Wave 7.1, we keep the smart pointer until reload or clear.
    }
}

void PluginHost::reloadPlugin(const std::string& pluginId) {
    unloadPlugin(pluginId);
    loadPlugin(pluginId);
}

void PluginHost::registerNativePlugin(std::unique_ptr<IURPGPlugin> plugin) {
    if (!plugin) return;
    const auto& info = plugin->getInfo();
    
    // Update or add to potential list
    auto it = std::find_if(m_availablePlugins.begin(), m_availablePlugins.end(),
        [&info](const PluginInfo& p) { return p.id == info.id; });

    if (it == m_availablePlugins.end()) {
        m_availablePlugins.push_back(info);
    } else {
        *it = info; // Sync state
    }

    m_activePlugins[info.id] = std::move(plugin);
}

IURPGPlugin* PluginHost::getActivePlugin(const std::string& id) const {
    auto it = m_activePlugins.find(id);
    if (it != m_activePlugins.end()) {
        return it->second.get();
    }
    return nullptr;
}

void PluginHost::tickActivePlugins(float deltaTime) {
    for (auto& [id, plugin] : m_activePlugins) {
        // Query status from available list
        auto it = std::find_if(m_availablePlugins.begin(), m_availablePlugins.end(),
            [&id](const PluginInfo& p) { return p.id == id; });

        if (it != m_availablePlugins.end() && it->status == PluginStatus::Active) {
            plugin->onTick(deltaTime);
        }
    }
}

} // namespace urpg::editor
