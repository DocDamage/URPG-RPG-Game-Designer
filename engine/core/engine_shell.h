#pragma once

#include "engine/core/scene/scene_manager.h"
#include "engine/core/input/input_core.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/sprite_batcher.h"
#include "engine/core/platform/platform_surface.h"
#include "engine/core/platform/renderer_backend.h"
#include "engine/core/clock.h"
#include <chrono>
#include <memory>
#include <atomic>

namespace urpg {

/**
 * @brief The central coordinator for the native game loop (Tick -> Update -> Render).
 * This shell ties together the SceneManager, InputCore, and Surface management.
 */
class EngineShell {
public:
    static EngineShell& getInstance() {
        static EngineShell instance;
        return instance;
    }

    /**
     * @brief Initializes engine systems with a platform surface and renderer.
     */
    bool startup(std::unique_ptr<IPlatformSurface> surface, std::unique_ptr<RendererBackend> renderer) {
        m_platform = std::move(surface);
        m_renderer = std::move(renderer);

        if (m_renderer && !m_renderer->initialize(m_platform.get())) {
            return false;
        }

        m_batcher = std::make_unique<SpriteBatcher>();
        m_clock.reset();
        m_isRunning = true;
        return true;
    }

    /**
     * @brief Performs a single engine tick.
     */
    void tick() {
        if (!m_isRunning) return;

        // 1. Poll Platform Events
        if (m_platform && !m_platform->pollEvents()) {
            shutdown();
            return;
        }

        // 2. Calculate Delta Time
        float dt = m_clock.getDelta();
        if (dt > 0.1f) dt = 0.016f; 

        // 3. Update active scene
        auto& sceneMgr = scene::SceneManager::getInstance();
        auto activeScene = sceneMgr.getActiveScene();
        if (activeScene) {
            activeScene->handleInput(m_inputCore);
            activeScene->onUpdate(dt);
        }

        // 4. Record and Render Frame
        if (m_renderer && m_batcher) {
            m_renderer->beginFrame();
            
            // Collect draw commands from scene
            m_batcher->begin();
            if (activeScene) {
                activeScene->draw(*m_batcher);
            }
            m_batcher->end();

            // Submit batches to the hardware backend
            m_renderer->renderBatches(m_batcher->getBatches());
            
            m_renderer->endFrame();
        }
    }

    void shutdown() {
        m_isRunning = false;
        if (m_renderer) {
            m_renderer->shutdown();
        }
        if (m_platform) {
            m_platform->shutdown();
        }
        RenderLayer::getInstance().flush();
    }

    bool isRunning() const { return m_isRunning; }
    input::InputCore& getInput() { return m_inputCore; }
    IPlatformSurface* getPlatform() { return m_platform.get(); }
    RendererBackend* getRenderer() { return m_renderer.get(); }

private:
    EngineShell() : m_isRunning(false) {}

    std::unique_ptr<IPlatformSurface> m_platform;
    std::unique_ptr<RendererBackend> m_renderer;
    std::unique_ptr<SpriteBatcher> m_batcher;
    std::atomic<bool> m_isRunning;
    input::InputCore m_inputCore;
    Clock m_clock;
};

} // namespace urpg
