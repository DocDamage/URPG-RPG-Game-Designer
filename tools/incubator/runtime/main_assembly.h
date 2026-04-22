#pragma once

#include "engine_assembly.h"
#include <chrono>
#include <iostream>

namespace urpg {

/**
 * @brief Main entry point logic for the URPG editor/engine.
 *
 * NOT CANONICAL: this synthetic loop is not the live runtime path.
 * Use EngineShell as the canonical top-level runtime entry point until this
 * compatibility scaffolding is either repaired or removed.
 */
class MainAssembly {
public:
    static int run(int argc, char** argv) {
        std::cout << "[URPG assembly] Booting URPG v3.1 Gold Release..." << std::endl;

        // Startup
        EngineAssembly::instance().startup();

        // Main Loop
        auto lastTime = std::chrono::high_resolution_clock::now();
        bool isRunning = true;

        while (isRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Frame Update
            EngineAssembly::instance().update(deltaTime);

            // Frame Render
            EngineAssembly::instance().render();

            // Termination Check (Synthetic for now)
            if (deltaTime > 100.0f) isRunning = false;

            // Limit loop for simulation
            static int frameCount = 0;
            if (++frameCount > 10) isRunning = false;
        }

        // Shutdown
        EngineAssembly::instance().shutdown();

        std::cout << "[URPG assembly] Graceful shutdown complete." << std::endl;
        return 0;
    }
};

} // namespace urpg
