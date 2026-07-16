#include <catch2/catch_test_macros.hpp>

#include <SDL2/SDL.h>

#include <string>

TEST_CASE("Vendored SDL exposes a functioning virtual game-controller stack",
          "[input][controller][sdl][capability][pcq650]") {
    INFO(SDL_GetError());
    REQUIRE(SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == 0);

    const int device_index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 2, 16, 0);
    INFO(SDL_GetError());
    REQUIRE(device_index >= 0);

    char guid[64]{};
    SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(device_index), guid, sizeof(guid));
    const std::string mapping = std::string(guid) +
        ",URPG Virtual Controller,a:b0,b:b1,x:b2,y:b3,back:b4,start:b6,leftshoulder:b9,"
        "rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,dpright:b14,leftx:a0,lefty:a1,";
    REQUIRE(SDL_GameControllerAddMapping(mapping.c_str()) >= 0);
    REQUIRE(SDL_IsGameController(device_index) == SDL_TRUE);

    SDL_GameController* controller = SDL_GameControllerOpen(device_index);
    INFO(SDL_GetError());
    REQUIRE(controller != nullptr);
    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
    REQUIRE(joystick != nullptr);
    const auto instance_id = SDL_JoystickInstanceID(joystick);
    REQUIRE(instance_id >= 0);

    SDL_GameControllerEventState(SDL_ENABLE);
    REQUIRE(SDL_JoystickSetVirtualButton(joystick, 0, 1) == 0);
    SDL_JoystickUpdate();
    SDL_PumpEvents();
    bool saw_press = false;
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.which == instance_id &&
            event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
            saw_press = true;
        }
    }
    REQUIRE(saw_press);

    REQUIRE(SDL_JoystickSetVirtualButton(joystick, 0, 0) == 0);
    SDL_GameControllerClose(controller);
    REQUIRE(SDL_JoystickDetachVirtual(device_index) == 0);
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);
}
