#include "sdl_surface.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "engine/core/input/input_core.h"

// Forward declaring SDL types to avoid pulling in SDL.h in the header
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace urpg {

static input::InputAction mapSdlKey(SDL_Keycode key) {
    switch (key) {
    case SDLK_UP:
    case SDLK_w:
        return input::InputAction::MoveUp;
    case SDLK_DOWN:
    case SDLK_s:
        return input::InputAction::MoveDown;
    case SDLK_LEFT:
    case SDLK_a:
        return input::InputAction::MoveLeft;
    case SDLK_RIGHT:
    case SDLK_d:
        return input::InputAction::MoveRight;
    case SDLK_z:
    case SDLK_RETURN:
    case SDLK_SPACE:
        return input::InputAction::Confirm;
    case SDLK_x:
    case SDLK_ESCAPE:
        return input::InputAction::Cancel;
    case SDLK_c:
    case SDLK_LSHIFT:
        return input::InputAction::Menu;
    case SDLK_BACKQUOTE:
        return input::InputAction::Debug;
    default:
        return input::InputAction::None;
    }
}

SDLSurface::SDLSurface(const std::string& title, int width, int height) {
    WindowConfig config;
    config.title = title;
    config.width = static_cast<uint32_t>(width);
    config.height = static_cast<uint32_t>(height);
    initialize(config);
}

bool SDLSurface::initialize(const WindowConfig& config) {
    if (m_isInitialized) {
        return true;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        diagnostics::RuntimeDiagnostics::error("platform.sdl", "sdl.initialize_failed",
                                               std::string("Failed to initialize SDL: ") + SDL_GetError());
        return false;
    }

    // Set OpenGL Attributes (TIER_BASIC: 3.3 Core)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                static_cast<int>(config.width), static_cast<int>(config.height),
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!m_window) {
        diagnostics::RuntimeDiagnostics::error("platform.sdl", "sdl.window_create_failed",
                                               std::string("Failed to create SDL window: ") + SDL_GetError());
        SDL_Quit();
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        diagnostics::RuntimeDiagnostics::error("platform.sdl", "sdl.gl_context_create_failed",
                                               std::string("Failed to create SDL GL context: ") + SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        SDL_Quit();
        return false;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);

    // Enable VSync (Adaptive if possible)
    if (SDL_GL_SetSwapInterval(-1) < 0) {
        SDL_GL_SetSwapInterval(1);
    }

    m_isInitialized = true;
    diagnostics::RuntimeDiagnostics::info("platform.sdl", "sdl.surface_initialized",
                                          "Surface initialized: " + std::to_string(config.width) + "x" +
                                              std::to_string(config.height));
    return true;
}

SDLSurface::~SDLSurface() {
    shutdown();
}

bool SDLSurface::pollEvents() {
    if (!m_isInitialized)
        return false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }

        // Window Resize events
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                // Future: Notify Renderer of resize
            }
        }

        // Input events (Keyboard/Controller)
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            auto action = mapSdlKey(event.key.keysym.sym);
            if (action != input::InputAction::None) {
                auto state = (event.type == SDL_KEYDOWN) ? input::ActionState::Pressed : input::ActionState::Released;
                EngineShell::getInstance().getInput().updateActionState(action, state);
            }
        }
    }

    return true;
}

void SDLSurface::present() {
    if (m_isInitialized && m_window) {
        SDL_GL_SwapWindow(m_window);
    }
}

void SDLSurface::shutdown() {
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_isInitialized) {
        SDL_Quit();
        m_isInitialized = false;
    }
}

} // namespace urpg
