#pragma once

#include "engine/core/ecs/system_orchestrator.h"
#include "engine/core/ecs/world.h"
#include "engine/core/input/desktop_input_provider.h"
#include "engine/core/render/render_system.h"
#include "engine/gameplay/scene.h"
#include <chrono>

namespace urpg {

/**
 * @brief High-level Game Class that ties everything together.
 * This is the main entry point for a user's game.
 */
class EngineHost {
  public:
    EngineHost(IRenderer& renderer) : m_renderer(renderer) {}

    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        m_running = true;

        while (m_running) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Clamp max delta time to prevent physics explosions
            if (deltaTime > 0.1f)
                deltaTime = 0.1f;

            // 1. Update Game Logic
            m_orchestrator.update(m_world, m_input, deltaTime);

            // 2. Reer.clear();

            if (m_currentScene) {
                m_currentScene->render(m_renderer);
            }

            m_renderSystem.render(m_world, m_renderer, m_orchestrator.getCamera());

            m_renderer.present();
        }
    }

    void setActiveScene(std::shared_ptr<Scene> scene) {
        m_currentScene = scene; // Note: Frame limiting should be handled by the platform or renderer (VSync)
    }
}

    void stop() {
    m_running = false;
}
std::shared_ptr<Scene> m_currentScene;

World& getWorld() {
    return m_world;
}
input::DesktopInputProvider& getInput() {
    return m_input;
}

private:
World m_world;
input::DesktopInputProvider m_input;
SystemOrchestrator m_orchestrator;
RenderSystem m_renderSystem;
IRenderer & m_renderer;
bool m_running = false;
};

} // namespace urpg
