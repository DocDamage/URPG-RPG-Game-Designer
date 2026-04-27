#include <catch2/catch_test_macros.hpp>
#include "engine/core/input/input_core.h"

using namespace urpg::input;

TEST_CASE("InputCore: Key Mapping and Action Processing", "[input][core]") {
    InputCore input;

    // Define some fictional key codes
    const int32_t KEY_UP = 38;
    const int32_t KEY_DOWN = 40;
    const int32_t KEY_Z = 90;

    input.mapKey(KEY_UP, InputAction::MoveUp);
    input.mapKey(KEY_DOWN, InputAction::MoveDown);
    input.mapKey(KEY_Z, InputAction::Confirm);

    SECTION("Key state tracking") {
        REQUIRE_FALSE(input.isActionActive(InputAction::MoveUp));

        input.processKeyEvent(KEY_UP, ActionState::Pressed);
        REQUIRE(input.isActionActive(InputAction::MoveUp));
        REQUIRE(input.isActionJustPressed(InputAction::MoveUp));

        input.endFrame();
        REQUIRE(input.isActionActive(InputAction::MoveUp));
        REQUIRE_FALSE(input.isActionJustPressed(InputAction::MoveUp));

        input.processKeyEvent(KEY_UP, ActionState::Released);
        REQUIRE_FALSE(input.isActionActive(InputAction::MoveUp));
        REQUIRE(input.isActionJustReleased(InputAction::MoveUp));

        input.endFrame();
        REQUIRE_FALSE(input.isActionActive(InputAction::MoveUp));
        REQUIRE_FALSE(input.isActionJustReleased(InputAction::MoveUp));
    }

    SECTION("Handler callbacks") {
        bool confirmTriggered = false;
        input.addHandler([&](InputAction action, ActionState state) {
            if (action == InputAction::Confirm && state == ActionState::Pressed) {
                confirmTriggered = true;
            }
        });

        input.processKeyEvent(KEY_Z, ActionState::Pressed);
        REQUIRE(confirmTriggered);
    }

    SECTION("Unmapped keys are ignored") {
        bool anyTriggered = false;
        input.addHandler([&](InputAction, ActionState) {
            anyTriggered = true;
        });

        // Key code 99 is not mapped
        input.processKeyEvent(99, ActionState::Pressed);
        REQUIRE_FALSE(anyTriggered);
    }

    SECTION("Repeated pressed events while held do not recreate a just-pressed edge") {
        int pressedEvents = 0;
        input.addHandler([&](InputAction action, ActionState state) {
            if (action == InputAction::Confirm && state == ActionState::Pressed) {
                ++pressedEvents;
            }
        });

        input.processKeyEvent(KEY_Z, ActionState::Pressed);
        REQUIRE(input.isActionJustPressed(InputAction::Confirm));
        REQUIRE(pressedEvents == 1);

        input.endFrame();
        REQUIRE(input.isActionActive(InputAction::Confirm));
        REQUIRE_FALSE(input.isActionJustPressed(InputAction::Confirm));

        input.processKeyEvent(KEY_Z, ActionState::Pressed);
        REQUIRE(input.isActionActive(InputAction::Confirm));
        REQUIRE_FALSE(input.isActionJustPressed(InputAction::Confirm));
        REQUIRE(pressedEvents == 1);
    }
}
