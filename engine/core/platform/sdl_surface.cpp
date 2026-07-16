#include "sdl_surface.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "engine/core/input/input_core.h"

// Forward declaring SDL types to avoid pulling in SDL.h in the header
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <optional>

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

static std::optional<action::ControllerButton> mapSdlControllerButton(Uint8 button) {
    using action::ControllerButton;
    switch (button) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return ControllerButton::DPadUp;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return ControllerButton::DPadDown;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return ControllerButton::DPadLeft;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return ControllerButton::DPadRight;
    case SDL_CONTROLLER_BUTTON_A: return ControllerButton::FaceBottom;
    case SDL_CONTROLLER_BUTTON_B: return ControllerButton::FaceRight;
    case SDL_CONTROLLER_BUTTON_X: return ControllerButton::FaceLeft;
    case SDL_CONTROLLER_BUTTON_Y: return ControllerButton::FaceTop;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return ControllerButton::LeftShoulder;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return ControllerButton::RightShoulder;
    case SDL_CONTROLLER_BUTTON_BACK: return ControllerButton::Select;
    case SDL_CONTROLLER_BUTTON_START: return ControllerButton::Start;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: return ControllerButton::LeftStickPress;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return ControllerButton::RightStickPress;
    default: return std::nullopt;
    }
}

static std::string controllerDeviceId(SDL_JoystickID instance_id) {
    return "sdl.controller." + std::to_string(instance_id);
}

static std::string controllerGlyphProfile(SDL_GameController* controller) {
    switch (SDL_GameControllerGetType(controller)) {
    case SDL_CONTROLLER_TYPE_PS3:
    case SDL_CONTROLLER_TYPE_PS4:
    case SDL_CONTROLLER_TYPE_PS5: return "playstation";
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO: return "switch";
    default: return "xbox";
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        diagnostics::RuntimeDiagnostics::error("platform.sdl", "sdl.initialize_failed",
                                               std::string("Failed to initialize SDL: ") + SDL_GetError());
        return false;
    }
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        diagnostics::RuntimeDiagnostics::warning("platform.sdl", "sdl.gamecontroller_initialize_failed",
                                                 std::string("Failed to initialize SDL game controller support: ") +
                                                     SDL_GetError());
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
    SDL_StartTextInput();
    for (int device_index = 0; device_index < SDL_NumJoysticks(); ++device_index) {
        openController(device_index);
    }
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
        if (m_eventCallback) {
            m_eventCallback(&event);
        }

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
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE) {
                EngineShell::getInstance().getInput().recordBackspace();
                continue;
            }
            auto action = mapSdlKey(event.key.keysym.sym);
            if (action != input::InputAction::None) {
                auto state = (event.type == SDL_KEYDOWN) ? input::ActionState::Pressed : input::ActionState::Released;
                EngineShell::getInstance().getInput().updateActionState(action, state);
            }
        }

        if (event.type == SDL_TEXTINPUT) {
            EngineShell::getInstance().getInput().appendTextInput(event.text.text);
        }

        if (event.type == SDL_TEXTEDITING) {
            EngineShell::getInstance().getInput().setTextEditing(event.edit.text);
        }

        if (event.type == SDL_CONTROLLERDEVICEADDED) {
            openController(event.cdevice.which);
        } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
            closeController(event.cdevice.which);
        } else if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
            const auto button = mapSdlControllerButton(event.cbutton.button);
            if (button) {
                (void)m_controllerInput.buttonEvent(
                    EngineShell::getInstance().getInput(), controllerDeviceId(event.cbutton.which), *button,
                    event.type == SDL_CONTROLLERBUTTONDOWN ? input::ActionState::Pressed
                                                           : input::ActionState::Released,
                    accessibility::InclusiveInputContext::Global);
            }
        } else if (event.type == SDL_CONTROLLERAXISMOTION &&
                   (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX ||
                    event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)) {
            const float raw_value = event.caxis.value < 0
                                        ? static_cast<float>(event.caxis.value) / 32768.0F
                                        : static_cast<float>(event.caxis.value) / 32767.0F;
            (void)m_controllerInput.axisEvent(
                EngineShell::getInstance().getInput(), controllerDeviceId(event.caxis.which),
                event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX ? input::ControllerAxis::LeftX
                                                               : input::ControllerAxis::LeftY,
                raw_value, accessibility::InclusiveInputContext::Global);
        }
    }

    return true;
}

void SDLSurface::openController(int device_index) {
    if (!SDL_IsGameController(device_index)) return;
    SDL_GameController* controller = SDL_GameControllerOpen(device_index);
    if (controller == nullptr) {
        diagnostics::RuntimeDiagnostics::warning("platform.sdl", "sdl.gamecontroller_open_failed",
                                                 std::string("Failed to open SDL game controller: ") + SDL_GetError());
        return;
    }
    const auto instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
    if (instance_id < 0 || m_gameControllers.contains(instance_id) ||
        !m_controllerInput.connect(controllerDeviceId(instance_id), controllerGlyphProfile(controller))) {
        SDL_GameControllerClose(controller);
        return;
    }
    m_gameControllers[instance_id] = controller;
    diagnostics::RuntimeDiagnostics::info("platform.sdl", "sdl.gamecontroller_connected",
                                          "Controller connected: " + controllerDeviceId(instance_id));
}

void SDLSurface::closeController(int32_t instance_id) {
    const auto found = m_gameControllers.find(instance_id);
    if (found == m_gameControllers.end()) return;
    (void)m_controllerInput.disconnect(EngineShell::getInstance().getInput(), controllerDeviceId(instance_id));
    SDL_GameControllerClose(static_cast<SDL_GameController*>(found->second));
    m_gameControllers.erase(found);
    diagnostics::RuntimeDiagnostics::info("platform.sdl", "sdl.gamecontroller_disconnected",
                                          "Controller disconnected: " + controllerDeviceId(instance_id));
}

void SDLSurface::present() {
    if (m_isInitialized && m_window) {
        SDL_GL_SwapWindow(m_window);
    }
}

void SDLSurface::shutdown() {
    while (!m_gameControllers.empty()) {
        closeController(m_gameControllers.begin()->first);
    }
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_isInitialized) {
        SDL_StopTextInput();
        SDL_Quit();
        m_isInitialized = false;
    }
}

} // namespace urpg
