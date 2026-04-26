#pragma once

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "platform_surface.h"

namespace urpg {

/**
 * @brief A "Headless" platform implementation for CI and logic-only testing.
 * This does not create a real window but simulates a fixed frame rate.
 */
class HeadlessSurface : public IPlatformSurface {
  public:
    bool initialize(const WindowConfig& config) override {
        diagnostics::RuntimeDiagnostics::info("platform.headless", "headless.surface_initialized",
                                              "Initializing Headless Surface: " + config.title + " (" +
                                                  std::to_string(config.width) + "x" + std::to_string(config.height) +
                                                  ")");
        m_config = config;
        m_quitRequested = false;
        return true;
    }

    bool pollEvents() override {
        // In headless mode, we don't have real OS events.
        // We could simulate script-based inputs here for automated testing.
        return !m_quitRequested;
    }

    void present() override {
        // No graphics output to swap.
        // Potentially used to log a frame summary or take a headless screenshot (snapshot test).
    }

    void shutdown() override {
        diagnostics::RuntimeDiagnostics::info("platform.headless", "headless.surface_shutdown",
                                              "Shutting down Headless Surface.");
    }

    void* getNativeHandle() const override { return nullptr; }

    void requestQuit() { m_quitRequested = true; }

  private:
    WindowConfig m_config;
    bool m_quitRequested = false;
};

} // namespace urpg
