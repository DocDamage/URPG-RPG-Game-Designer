#pragma once

#include "engine/core/platform/platform_surface.h"
#include <string>

// SDL2 is required for this implementation.
struct SDL_Window;

namespace urpg {

struct SdlSurfaceProbeResult {
    bool videoInitialized = false;
    bool controllerInitialized = false;
    bool windowCreated = false;
    bool glContextCreated = false;
    std::string videoError;
    std::string controllerError;
    std::string windowError;
    std::string glContextError;
};

/**
 * @brief SDL2-based implementation of the platform surface.
 * Handles window creation, event polling, and OpenGL context binding.
 */
class SDLSurface : public IPlatformSurface {
  public:
    SDLSurface() = default;
    SDLSurface(const std::string& title, int width, int height);
    virtual ~SDLSurface();

    bool initialize(const WindowConfig& config) override;
    bool pollEvents() override;
    void present() override;
    void show() override;
    void shutdown() override;
    void* getNativeHandle() const override { return m_window; }

    // SDL-specific accessors
    SDL_Window* getNativeWindow() const { return m_window; }
    static SdlSurfaceProbeResult probe(const WindowConfig& config, bool createOpenGlWindow);

  private:
    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr;
    bool m_isInitialized = false;
};

} // namespace urpg
