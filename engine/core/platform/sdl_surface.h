#pragma once

#include "engine/core/platform/platform_surface.h"
#include <string>

// SDL2 is required for this implementation.
struct SDL_Window;

namespace urpg {

/**
 * @brief SDL2-based implementation of the platform surface.
 * Handles window creation, event polling, and OpenGL context binding.
 */
class SDLSurface : public IPlatformSurface {
public:
    SDLSurface(const std::string& title, int width, int height);
    virtual ~SDLSurface();

    bool pollEvents() override;
    void present() override;
    void shutdown() override;

    // SDL-specific accessors
    SDL_Window* getNativeWindow() const { return m_window; }

private:
    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr;
    bool m_isInitialized = false;
};

} // namespace urpg
