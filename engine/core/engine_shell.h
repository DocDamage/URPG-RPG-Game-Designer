#pragma once

#include "engine/core/scene/scene_manager.h"
#include "engine/core/input/input_core.h"
#include "engine/core/render/render_layer.h"
#include <chrono>
#include <memory>
#include <atomic>

namespace urpg {

/**
 * @brief The central coordinator for the native game loop (Tick -> Update -> Render).
 * This shell ties together the SceneManager, InputCore, and RenderLayer.
 */
class EngineShell {
public:
    static EngineShell& getInstance() {
        static EngineShell instance;
        return instance;
    }

    /**
     * @brief Initializes engine systems.
     */
    void startup() {
        m_isRunning = true;
        m_lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Performs a single engine tick.
     * This would be called by the platform-specific entry point (WinMain/main).
     */
    void tick() {
        if (!m_isRunning) return;

        // 1. Calculate Delta Time
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - m_lastFrameTime;
        float dt = elapsed.count();
        
        // Prevent huge jumps (e.g., after a breakpoint/pause)
        if (dt > 0.1f) dt = 0.016f; 
        
        m_lastFrameTime = currentTime;

        // 2. Scene Logic & Input Integration
        auto& sceneMgr = scene::SceneManager::getInstance();
        auto activeScene = sceneMgr.getActiveScene();
        
        if (activeScene) {
            // High-level scene orchestration: Coordinate Input and Update
            activeScene->handleInput(m_inputCore);
            activeScene->onUpdate(dt);
        }

        // 3. Render Submission handled by scenes via RenderLayer
        // After this tick, the platform renderer would flush RenderLayer::getInstance().
    }

    void shutdown() {
        m_isRunning = false;
        RenderLayer::getInstance().flush();
    }

    bool isRunning() const { return m_isRunning; }
    input::InputCore& getInput() { return m_inputCore; }

private:
    EngineShell() : m_isRunning(false) {}

    std::atomic<bool> m_isRunning;
    input::InputCore m_inputCore;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTime;
};

} // namespace urpg
