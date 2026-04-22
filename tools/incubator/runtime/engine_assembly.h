#pragma once

#include "../../../editor/editor_shell.h"
#include "../editor/plugin_host.h"
#include "../editor/script_bridge.h"
#include "../../../engine/core/render/renderer_backend.h"
#include "../../../engine/core/threading/thread_roles.h"

namespace urpg {

/**
 * @brief Bootstraps the entire URPG engine and editor workspace.
 *
 * NOT CANONICAL: this stale assembly seam is not wired into the live build/runtime path.
 * Use EngineShell as the canonical top-level runtime entry point until this header is
 * either repaired or removed.
 */
class EngineAssembly {
public:
    static EngineAssembly& instance() {
        static EngineAssembly inst;
        return inst;
    }

    /**
     * @brief Initializes all subsystems in the correct dependency order.
     */
    void startup() {
        // 1. Core Threading
        core::threading::ThreadRoles::instance().initialize();

        // 2. Scripting Bridge (JS)
        editor::ScriptBridge::instance().startup();

        // 3. Plugin Host (Native)
        editor::PluginHost::instance().discoverPlugins("plugins/");

        // 4. Editor Shell (UI)
        editor::EditorShell::instance().startup();

        m_isInitialized = true;
    }

    /**
     * @brief Shuts down all subsystems gracefully.
     */
    void shutdown() {
        editor::EditorShell::instance().shutdown();
        editor::ScriptBridge::instance().shutdown();
        core::threading::ThreadRoles::instance().shutdown();
        m_isInitialized = false;
    }

    /**
     * @brief Main update loop coordinator.
     */
    void update(float deltaTime) {
        if (!m_isInitialized) return;

        // Tick plugins first (logic)
        editor::PluginHost::instance().tickActivePlugins(deltaTime);

        // Update editor workspace
        editor::EditorShell::instance().update(deltaTime);
    }

    /**
     * @brief Main render loop coordinator.
     */
    void render() {
        if (!m_isInitialized) return;

        // Render editor panels
        editor::EditorShell::instance().render();
    }

    bool isInitialized() const { return m_isInitialized; }

private:
    EngineAssembly() : m_isInitialized(false) {}
    bool m_isInitialized;
};

} // namespace urpg
