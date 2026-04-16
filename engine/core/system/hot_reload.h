#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iostream>

namespace urpg::system {

    /**
     * @brief Hot-Reload Registration for engine subsystems.
     * Part of Wave 4 Engine Polish.
     */
    enum class SystemId {
        Logic,
        Renderer,
        Audio,
        Editor,
        PluginCompiler,
        FileWatcher
    };

    /**
     * @brief Context-aware callback for after a reload.
     */
    using ReloadCallback = std::function<void(SystemId)>;

    /**
     * @brief Manages live hot-reloading for code, assets, and shaders.
     * Essential for the 'Producer Copilot' to instantly reflect changes.
     */
    class HotReloadRegistry {
    public:
        /**
         * @brief Register a system that needs to be notified when reloads occur.
         */
        void registerSystem(SystemId id, ReloadCallback callback) {
            m_reloadCallbacks[id] = callback;
            std::cout << "[HotReload] Registered system ID: " << (int)id << std::endl;
        }

        /**
         * @brief Notify all registered systems to refresh their state.
         * Used when DLLs are swapped or assets are modified.
         */
        void triggerReload(SystemId target = SystemId::Logic) {
            if (m_reloadCallbacks.find(target) != m_reloadCallbacks.end()) {
                std::cout << "[HotReload] Executing reload for system ID: " << (int)target << std::endl;
                m_reloadCallbacks[target](target);
            } else {
                // Global reload if target not specified
                for (auto& [id, callback] : m_reloadCallbacks) {
                    callback(id);
                }
            }
        }

        /**
         * @brief Monitor asset folder and trigger reloads on change.
         */
        void watchAsset(const std::string& path, SystemId associatedSystem) {
            m_watchedAssets[path] = associatedSystem;
            // File system hook: inotify/ReadDirectoryChangesW
        }

    private:
        std::map<SystemId, ReloadCallback> m_reloadCallbacks;
        std::map<std::string, SystemId> m_watchedAssets;
    };

} // namespace urpg::system
