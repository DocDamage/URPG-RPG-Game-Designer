// InputManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Input/TouchInput Compatibility Surface

#include "runtimes/compat_js/input_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace urpg::compat;

TEST_CASE("InputManager: Initialization", "[input_manager]") {
    InputManager& im = InputManager::instance();
    
    SECTION("Initialize clears state") {
        im.initialize();
        
        REQUIRE(im.getDir4() == 0);
        REQUIRE(im.getDir8() == 0);
        
        im.shutdown();
    }
    
    SECTION("Clear resets all state") {
        im.initialize();
        im.clear();
        
        REQUIRE(im.getDir4() == 0);
        REQUIRE(im.getDir8() == 0);
        
        im.shutdown();
    }
}

TEST_CASE("InputManager: Key state queries", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("isPressed returns false for unpressed key") {
        REQUIRE_FALSE(im.isPressed(InputKey::DOWN));
        REQUIRE_FALSE(im.isPressed(InputKey::UP));
    }
    
    SECTION("isTriggered returns false for untriggered key") {
        REQUIRE_FALSE(im.isTriggered(InputKey::DECISION));
        REQUIRE_FALSE(im.isTriggered(InputKey::CANCEL));
    }
    
    SECTION("isRepeated returns false for unrepeated key") {
        REQUIRE_FALSE(im.isRepeated(InputKey::SHIFT));
    }
    
    SECTION("isReleased returns false for unreleased key") {
        REQUIRE_FALSE(im.isReleased(InputKey::CONTROL));
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Direction state", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Initial dir4 is 0") {
        REQUIRE(im.getDir4() == 0);
    }
    
    SECTION("Initial dir8 is 0") {
        REQUIRE(im.getDir8() == 0);
    }
    
    SECTION("isDirectionPressed returns false initially") {
        REQUIRE_FALSE(im.isDirectionPressed(2));
        REQUIRE_FALSE(im.isDirectionPressed(4));
        REQUIRE_FALSE(im.isDirectionPressed(6));
        REQUIRE_FALSE(im.isDirectionPressed(8));
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Mouse state", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Initial mouse position is 0") {
        REQUIRE(im.mouseX() == 0);
        REQUIRE(im.mouseY() == 0);
    }
    
    SECTION("Initial mouse button state is false") {
        REQUIRE_FALSE(im.isMousePressed(0));
        REQUIRE_FALSE(im.isMouseTriggered(0));
    }
    
    SECTION("Initial mouse wheel is 0") {
        REQUIRE(im.mouseWheel() == 0);
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Touch state", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Initial touch position is 0") {
        REQUIRE(im.touchX() == 0);
        REQUIRE(im.touchY() == 0);
    }
    
    SECTION("Initial touch state is false") {
        REQUIRE_FALSE(im.isTouchPressed());
        REQUIRE_FALSE(im.isTouchTriggered());
    }
    
    SECTION("Initial touch count is 0") {
        REQUIRE(im.touchCount() == 0);
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Gamepad state", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Initial gamepad state is disconnected") {
        REQUIRE_FALSE(im.isGamepadConnected());
    }
    
    SECTION("Gamepad button state is false initially") {
        REQUIRE_FALSE(im.isGamepadButtonPressed(0));
        REQUIRE_FALSE(im.isGamepadButtonTriggered(0));
    }
    
    SECTION("Gamepad axis returns 0 initially") {
        REQUIRE(im.gamepadAxis(0) == Approx(0.0));
        REQUIRE(im.gamepadAxis(1) == Approx(0.0));
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Action mappings", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Map key to action") {
        im.mapKeyToAction("confirm", InputKey::DECISION);
        
        // Action should be mapped
        REQUIRE_FALSE(im.isActionPressed("confirm"));
    }
    
    SECTION("Map gamepad button to action") {
        im.mapGamepadButtonToAction("confirm", InputKey::GAMEPAD_A);
        
        // Action should be mapped
        REQUIRE_FALSE(im.isActionPressed("confirm"));
    }
    
    SECTION("Unmap action") {
        im.mapKeyToAction("cancel", InputKey::CANCEL);
        im.unmapAction("cancel");
        
        // Action should be unmapped
    }
    
    SECTION("Clear action mappings") {
        im.mapKeyToAction("test", InputKey::SHIFT);
        im.clearActionMappings();
        
        // All mappings should be cleared
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Update cycle", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();
    
    SECTION("Update clears triggered states") {
        im.update();
        
        // After update, triggered states should be cleared
        REQUIRE_FALSE(im.isTriggered(InputKey::DECISION));
    }
    
    SECTION("Update clears mouse wheel") {
        im.update();
        
        REQUIRE(im.mouseWheel() == 0);
    }
    
    im.shutdown();
}

TEST_CASE("InputManager: Method status registry", "[input_manager]") {
    InputManager& im = InputManager::instance();
    
    SECTION("GetMethodStatus returns FULL for core methods") {
        CompatStatus status = im.getMethodStatus("isPressed");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns FULL for direction methods") {
        CompatStatus status = im.getMethodStatus("dir4");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns UNSUPPORTED for unknown methods") {
        CompatStatus status = im.getMethodStatus("nonexistentMethod");
        REQUIRE(status == CompatStatus::UNSUPPORTED);
    }
}

TEST_CASE("TouchInput: Static interface", "[input_manager]") {
    TouchInput::initialize();
    
    SECTION("Initial X position is 0") {
        REQUIRE(TouchInput::getX() == 0);
    }
    
    SECTION("Initial Y position is 0") {
        REQUIRE(TouchInput::getY() == 0);
    }
    
    SECTION("Initial pressed state is false") {
        REQUIRE_FALSE(TouchInput::isPressed());
    }
    
    SECTION("Initial triggered state is false") {
        REQUIRE_FALSE(TouchInput::isTriggered());
    }
    
    SECTION("Initial wheel is 0") {
        REQUIRE(TouchInput::getWheel() == 0);
    }
    
    TouchInput::shutdown();
}

TEST_CASE("TouchInput: Method status registry", "[input_manager]") {
    SECTION("GetMethodStatus returns FULL for core methods") {
        CompatStatus status = TouchInput::getMethodStatus("getX");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns FULL for pressed state") {
        CompatStatus status = TouchInput::getMethodStatus("isPressed");
        REQUIRE(status == CompatStatus::FULL);
    }
}

TEST_CASE("InputKey: Constants are MZ compatible", "[input_manager]") {
    SECTION("Direction keys match MZ values") {
        REQUIRE(InputKey::DOWN == 2);
        REQUIRE(InputKey::LEFT == 4);
        REQUIRE(InputKey::RIGHT == 6);
        REQUIRE(InputKey::UP == 8);
    }
    
    SECTION("Action keys are defined") {
        REQUIRE(InputKey::DECISION == 13);
        REQUIRE(InputKey::CANCEL == 27);
        REQUIRE(InputKey::SHIFT == 16);
        REQUIRE(InputKey::CONTROL == 17);
    }
    
    SECTION("Function keys are sequential") {
        REQUIRE(InputKey::F1 == 112);
        REQUIRE(InputKey::F12 == 123);
    }
    
    SECTION("Gamepad buttons are defined") {
        REQUIRE(InputKey::GAMEPAD_A == 1000);
        REQUIRE(InputKey::GAMEPAD_B == 1001);
        REQUIRE(InputKey::GAMEPAD_START == 1009);
    }
}

TEST_CASE("ActionMapping: Structure", "[input_manager]") {
    ActionMapping mapping;
    
    SECTION("Default values") {
        REQUIRE(mapping.actionId.empty());
        REQUIRE(mapping.keyCodes.empty());
        REQUIRE(mapping.gamepadButtons.empty());
        REQUIRE(mapping.gamepadAxis == -1);
        REQUIRE(mapping.axisThreshold == Approx(0.5));
    }
    
    SECTION("Can set action ID") {
        mapping.actionId = "test_action";
        REQUIRE(mapping.actionId == "test_action");
    }
    
    SECTION("Can add key codes") {
        mapping.keyCodes.push_back(InputKey::DECISION);
        mapping.keyCodes.push_back(InputKey::GAMEPAD_A);
        REQUIRE(mapping.keyCodes.size() == 2);
    }
}

TEST_CASE("InputState: Structure defaults", "[input_manager]") {
    InputState state;
    
    SECTION("Keyboard arrays are zeroed") {
        for (int i = 0; i < 256; ++i) {
            REQUIRE_FALSE(state.keys[i]);
            REQUIRE_FALSE(state.prevKeys[i]);
            REQUIRE_FALSE(state.triggeredKeys[i]);
            REQUIRE_FALSE(state.repeatedKeys[i]);
            REQUIRE(state.repeatCounters[i] == 0);
        }
    }
    
    SECTION("Mouse state is zeroed") {
        REQUIRE(state.mouseX == 0);
        REQUIRE(state.mouseY == 0);
        REQUIRE(state.mouseWheel == 0);
        for (int i = 0; i < 3; ++i) {
            REQUIRE_FALSE(state.mousePressed[i]);
            REQUIRE_FALSE(state.mouseTriggered[i]);
        }
    }
    
    SECTION("Touch state is zeroed") {
        REQUIRE(state.touchX == 0);
        REQUIRE(state.touchY == 0);
        REQUIRE_FALSE(state.touchPressed);
        REQUIRE_FALSE(state.touchTriggered);
        REQUIRE(state.touchCount == 0);
    }
    
    SECTION("Gamepad state is zeroed") {
        REQUIRE_FALSE(state.gamepadConnected);
        REQUIRE(state.gamepadId == -1);
        for (int i = 0; i < 16; ++i) {
            REQUIRE_FALSE(state.gamepadButtons[i]);
            REQUIRE_FALSE(state.gamepadButtonsTriggered[i]);
        }
        for (int i = 0; i < 4; ++i) {
            REQUIRE(state.gamepadAxes[i] == Approx(0.0));
        }
    }
    
    SECTION("Direction state is zeroed") {
        REQUIRE(state.dir4 == 0);
        REQUIRE(state.dir8 == 0);
    }
}
