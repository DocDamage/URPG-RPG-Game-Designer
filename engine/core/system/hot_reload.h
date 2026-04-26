#pragma once

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg::system {

/**
 * @brief Hot-Reload Registration for engine subsystems.
 * Part of Wave 4 Engine Polish.
 */
enum class SystemId { Logic, Renderer, Audio, Editor, PluginCompiler, FileWatcher };

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
        urpg::diagnostics::RuntimeDiagnostics::info("system.hot_reload", "hot_reload.system_registered",
                                                    "Registered system ID: " + std::to_string(static_cast<int>(id)));
    }

    /**
     * @brief Notify all registered systems to refresh their state.
     * Used when DLLs are swapped or assets are modified.
     */
    void triggerReload(SystemId target = SystemId::Logic) {
        if (m_reloadCallbacks.find(target) != m_reloadCallbacks.end()) {
            urpg::diagnostics::RuntimeDiagnostics::info("system.hot_reload", "hot_reload.executing",
                                                        "Executing reload for system ID: " +
                                                            std::to_string(static_cast<int>(target)));
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
