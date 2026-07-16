#pragma once

#include "engine/core/input/controller_input_provider.h"
#include "engine/core/platform/platform_surface.h"
#include <cstdint>
#include <functional>
#include <map>
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
    using EventCallback = std::function<void(const void*)>;

    SDLSurface() = default;
    SDLSurface(const std::string& title, int width, int height);
    virtual ~SDLSurface();

    bool initialize(const WindowConfig& config) override;
    bool pollEvents() override;
    void present() override;
    void shutdown() override;
    void* getNativeHandle() const override { return m_window; }

    // SDL-specific accessors
    SDL_Window* getNativeWindow() const { return m_window; }
    void* getNativeGlContext() const { return m_glContext; }
    void setEventCallback(EventCallback callback) { m_eventCallback = std::move(callback); }
    bool setControllerBindings(const action::ControllerBindingRuntime& bindings) {
        return m_controllerInput.applyBindings(bindings);
    }
    size_t connectedControllerCount() const { return m_controllerInput.connectedDeviceCount(); }

  private:
    void openController(int device_index);
    void closeController(int32_t instance_id);

    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr;
    EventCallback m_eventCallback;
    input::ControllerInputProvider m_controllerInput;
    std::map<int32_t, void*> m_gameControllers;
    bool m_isInitialized = false;
};

} // namespace urpg
