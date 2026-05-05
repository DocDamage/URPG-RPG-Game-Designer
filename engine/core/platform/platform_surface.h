#pragma once

#include <string>
#include <cstdint>
#include <functional>

namespace urpg {

/**
 * @brief Configuration for the platform window.
 */
struct WindowConfig {
    std::string title = "URPG Engine";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool fullscreen = false;
    bool resizable = true;
    bool hidden = false;
};

/**
 * @brief Interface for platform-specific window and event handling.
 * This allows the engine to remain agnostic of SDL2, Win32, or GLFW.
 */
class IPlatformSurface {
public:
    virtual ~IPlatformSurface() = default;

    /**
     * @brief Initializes the window and rendering context.
     */
    virtual bool initialize(const WindowConfig& config) = 0;

    /**
     * @brief Polls OS events (mouse, keyboard, window) and pumps them into EngineShell.
     * @return False if the application should quit.
     */
    virtual bool pollEvents() = 0;

    /**
     * @brief Swaps the back buffer to the front (displaying the frame).
     */
    virtual void present() = 0;

    /**
     * @brief Shows the native window after deferred startup. Headless surfaces ignore this.
     */
    virtual void show() {}

    /**
     * @brief Shuts down the windowing system.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Returns the native window handle if needed (HWND, SDL_Window*, etc).
     */
    virtual void* getNativeHandle() const = 0;
};

} // namespace urpg
