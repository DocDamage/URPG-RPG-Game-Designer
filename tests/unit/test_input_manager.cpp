#include "runtimes/compat_js/input_manager.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;
using urpg::Value;

TEST_CASE("InputManager: initialization and clear", "[input_manager]") {
    InputManager& im = InputManager::instance();

    im.initialize();
    REQUIRE(im.getDir4() == 0);
    REQUIRE(im.getDir8() == 0);

    im.setMousePosition(12, 34);
    im.clear();
    REQUIRE(im.getMouseX() == 0);
    REQUIRE(im.getMouseY() == 0);

    im.shutdown();
}

TEST_CASE("InputManager: key state transitions", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();

    REQUIRE_FALSE(im.isPressed(InputKey::DECISION));
    REQUIRE_FALSE(im.isTriggered(InputKey::DECISION));

    im.setKeyPressed(InputKey::DECISION, true);
    REQUIRE(im.isPressed(InputKey::DECISION));
    REQUIRE(im.isTriggered(InputKey::DECISION));

    im.update();
    REQUIRE(im.isPressed(InputKey::DECISION));
    REQUIRE(im.isTriggered(InputKey::DECISION));

    im.update();
    REQUIRE_FALSE(im.isTriggered(InputKey::DECISION));

    im.setKeyPressed(InputKey::DECISION, false);
    REQUIRE_FALSE(im.isPressed(InputKey::DECISION));
    REQUIRE(im.isReleased(InputKey::DECISION));

    im.shutdown();
}

TEST_CASE("InputManager: direction state", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();

    REQUIRE(im.getDir4() == 0);
    REQUIRE(im.getDir8() == 0);

    im.setKeyPressed(InputKey::DOWN, true);
    im.update();
    REQUIRE(im.getDir4() == 2);
    REQUIRE(im.isDirectionPressed(2));

    im.setKeyPressed(InputKey::DOWN, false);
    im.update();
    REQUIRE(im.getDir4() == 0);

    im.shutdown();
}

TEST_CASE("InputManager: mouse and touch access", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();

    im.setMousePosition(100, 200);
    REQUIRE(im.getMouseX() == 100);
    REQUIRE(im.getMouseY() == 200);

    im.setMousePressed(0, true);
    REQUIRE(im.isMousePressed(0));
    REQUIRE(im.isMouseTriggered(0));

    im.setMouseWheel(3);
    REQUIRE(im.getMouseWheel() == 3);
    im.update();
    REQUIRE_FALSE(im.isMouseTriggered(0));
    REQUIRE(im.getMouseWheel() == 0);

    im.setTouchPosition(50, 60);
    REQUIRE(im.getTouchX() == 50);
    REQUIRE(im.getTouchY() == 60);

    im.setTouchPressed(true);
    REQUIRE(im.isTouchPressed());
    REQUIRE(im.isTouchTriggered());
    REQUIRE(im.getTouchCount() == 1);

    im.shutdown();
}

TEST_CASE("InputManager: mouse trigger is a one-frame edge", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();

    im.setMousePressed(0, true);
    REQUIRE(im.isMouseTriggered(0));

    im.update();
    REQUIRE(im.isMousePressed(0));
    REQUIRE_FALSE(im.isMouseTriggered(0));

    im.setMousePressed(0, false);
    im.setMousePressed(0, true);
    REQUIRE(im.isMouseTriggered(0));

    im.shutdown();
}

TEST_CASE("InputManager: gamepad and action mapping", "[input_manager]") {
    InputManager& im = InputManager::instance();
    im.initialize();

    im.setGamepadConnected(true, 1);
    REQUIRE(im.isGamepadConnected());
    REQUIRE(im.getGamepadId() == 1);

    im.setGamepadAxis(0, 0.75);
    REQUIRE(im.getGamepadAxis(0) == Catch::Approx(0.75));

    im.setGamepadButton(0, true);
    REQUIRE(im.isGamepadButtonTriggered(0));
    im.update();
    REQUIRE(im.isGamepadButtonPressed(0));
    REQUIRE_FALSE(im.isGamepadButtonTriggered(0));

    im.mapKeyToAction(InputKey::DECISION, "confirm");
    const ActionMapping* mapping = im.getActionMapping("confirm");
    REQUIRE(mapping != nullptr);
    REQUIRE(mapping->actionId == "confirm");

    im.setKeyPressed(InputKey::DECISION, true);
    REQUIRE(im.isActionPressed("confirm"));

    im.unmapAction("confirm");
    REQUIRE(im.getActionMapping("confirm") == nullptr);

    im.shutdown();
}

TEST_CASE("InputManager: method status registry", "[input_manager]") {
    InputManager& im = InputManager::instance();
    (void)im;

    REQUIRE(InputManager::getMethodStatus("isPressed") == CompatStatus::PARTIAL);
    REQUIRE(InputManager::getMethodStatus("dir4") == CompatStatus::PARTIAL);
    REQUIRE(InputManager::getMethodStatus("initialize") == CompatStatus::PARTIAL);
    REQUIRE(InputManager::getMethodStatus("clear") == CompatStatus::PARTIAL);
    REQUIRE(InputManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("TouchInput: instance interface", "[input_manager]") {
    TouchInput& touch = TouchInput::instance();
    touch.clear();
    touch.resetCameraTransform();

    REQUIRE(touch.getX() == 0);
    REQUIRE(touch.getY() == 0);
    REQUIRE_FALSE(touch.isPressed());
    REQUIRE_FALSE(touch.isTriggered());

    touch.setTouchPosition(9, 11);
    touch.setTouchPressed(true);
    REQUIRE(touch.getX() == 9);
    REQUIRE(touch.getY() == 11);
    REQUIRE(touch.getWorldX() == 9);
    REQUIRE(touch.getWorldY() == 11);
    REQUIRE(touch.isPressed());
    REQUIRE(touch.isTriggered());

    touch.update();
    REQUIRE_FALSE(touch.isTriggered());

    touch.setTouchPressed(false);
    REQUIRE(touch.isReleased());
}

TEST_CASE("TouchInput: world coordinates apply camera transform", "[input_manager]") {
    TouchInput& touch = TouchInput::instance();
    touch.clear();
    touch.resetCameraTransform();

    touch.setTouchPosition(250, 110);
    touch.setCameraTransform(2.0, 0.5, 50, 10);

    // (250 - 50) / 2.0 = 100
    // (110 - 10) / 0.5 = 200
    REQUIRE(touch.getWorldX() == 100);
    REQUIRE(touch.getWorldY() == 200);

    touch.resetCameraTransform();
    REQUIRE(touch.getWorldX() == 250);
    REQUIRE(touch.getWorldY() == 110);
}

TEST_CASE("TouchInput: movement speed and tap counting", "[input_manager]") {
    TouchInput& touch = TouchInput::instance();
    touch.clear();
    touch.resetCameraTransform();

    touch.setTouchPosition(0, 0);
    touch.setTouchPressed(true);
    touch.update();

    touch.setTouchPosition(3, 4);
    touch.update();
    REQUIRE(touch.getMoveSpeed() == Catch::Approx(5.0));
    REQUIRE(touch.getMoveDistance() == Catch::Approx(5.0));

    touch.setTouchPressed(false);
    touch.update();
    REQUIRE(touch.getTapCount() == 1);
}

TEST_CASE("TouchInput: method status and constants", "[input_manager]") {
    TouchInput& touch = TouchInput::instance();
    (void)touch;

    REQUIRE(TouchInput::getMethodStatus("x") == CompatStatus::PARTIAL);
    REQUIRE(TouchInput::getMethodStatus("isPressed") == CompatStatus::PARTIAL);
    REQUIRE(TouchInput::getMethodStatus("worldX") == CompatStatus::PARTIAL);
    REQUIRE(TouchInput::getMethodStatus("worldY") == CompatStatus::PARTIAL);
    REQUIRE(TouchInput::getMethodStatus("moveSpeed") == CompatStatus::PARTIAL);
    REQUIRE(TouchInput::getMethodStatus("clear") == CompatStatus::PARTIAL);

    REQUIRE(InputKey::DOWN == 2);
    REQUIRE(InputKey::LEFT == 4);
    REQUIRE(InputKey::RIGHT == 6);
    REQUIRE(InputKey::UP == 8);

    REQUIRE(InputKey::DECISION == 13);
    REQUIRE(InputKey::CANCEL == 27);
    REQUIRE(InputKey::F1 == 112);
    REQUIRE(InputKey::F12 == 123);
    REQUIRE(InputKey::GAMEPAD_A == 1000);
}

TEST_CASE("InputManager: registerAPI routes to runtime state", "[input_manager]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    InputManager::registerAPI(ctx);

    InputManager& im = InputManager::instance();
    im.initialize();
    im.clear();

    im.setKeyPressed(InputKey::DECISION, true);
    const auto pressed = ctx.callMethod("Input", "isPressed", {Value::Int(InputKey::DECISION)});
    REQUIRE(pressed.success);
    REQUIRE(std::holds_alternative<int64_t>(pressed.value.v));
    REQUIRE(std::get<int64_t>(pressed.value.v) == 1);

    const auto update = ctx.callMethod("Input", "update", {});
    REQUIRE(update.success);

    im.setKeyPressed(InputKey::DOWN, true);
    REQUIRE(ctx.callMethod("Input", "update", {}).success);
    const auto dir4 = ctx.callMethod("Input", "dir4", {});
    REQUIRE(dir4.success);
    REQUIRE(std::holds_alternative<int64_t>(dir4.value.v));
    REQUIRE(std::get<int64_t>(dir4.value.v) == 2);

    im.setKeyPressed(InputKey::DECISION, false);
    const auto released = ctx.callMethod("Input", "isReleased", {Value::Int(InputKey::DECISION)});
    REQUIRE(released.success);
    REQUIRE(std::holds_alternative<int64_t>(released.value.v));
    REQUIRE(std::get<int64_t>(released.value.v) == 1);

    REQUIRE(ctx.callMethod("Input", "clear", {}).success);
    const auto pressedAfterClear = ctx.callMethod("Input", "isPressed", {Value::Int(InputKey::DOWN)});
    REQUIRE(pressedAfterClear.success);
    REQUIRE(std::holds_alternative<int64_t>(pressedAfterClear.value.v));
    REQUIRE(std::get<int64_t>(pressedAfterClear.value.v) == 0);

    im.shutdown();
}

TEST_CASE("TouchInput: registerAPI routes to runtime state", "[input_manager]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    TouchInput::registerAPI(ctx);

    TouchInput& touch = TouchInput::instance();
    touch.clear();
    touch.resetCameraTransform();
    touch.setTouchPosition(10, 20);
    touch.setTouchPressed(true);

    const auto x = ctx.callMethod("TouchInput", "x", {});
    REQUIRE(x.success);
    REQUIRE(std::holds_alternative<int64_t>(x.value.v));
    REQUIRE(std::get<int64_t>(x.value.v) == 10);

    const auto y = ctx.callMethod("TouchInput", "y", {});
    REQUIRE(y.success);
    REQUIRE(std::holds_alternative<int64_t>(y.value.v));
    REQUIRE(std::get<int64_t>(y.value.v) == 20);

    const auto triggered = ctx.callMethod("TouchInput", "isTriggered", {});
    REQUIRE(triggered.success);
    REQUIRE(std::holds_alternative<int64_t>(triggered.value.v));
    REQUIRE(std::get<int64_t>(triggered.value.v) == 1);

    REQUIRE(ctx.callMethod("TouchInput", "update", {}).success);
    const auto triggeredAfterUpdate = ctx.callMethod("TouchInput", "isTriggered", {});
    REQUIRE(triggeredAfterUpdate.success);
    REQUIRE(std::holds_alternative<int64_t>(triggeredAfterUpdate.value.v));
    REQUIRE(std::get<int64_t>(triggeredAfterUpdate.value.v) == 0);

    touch.setTouchPressed(false);
    const auto released = ctx.callMethod("TouchInput", "isReleased", {});
    REQUIRE(released.success);
    REQUIRE(std::holds_alternative<int64_t>(released.value.v));
    REQUIRE(std::get<int64_t>(released.value.v) == 1);

    REQUIRE(ctx.callMethod("TouchInput", "clear", {}).success);
    const auto xAfterClear = ctx.callMethod("TouchInput", "x", {});
    REQUIRE(xAfterClear.success);
    REQUIRE(std::holds_alternative<int64_t>(xAfterClear.value.v));
    REQUIRE(std::get<int64_t>(xAfterClear.value.v) == 0);
}

TEST_CASE("Input structs: defaults", "[input_manager]") {
    ActionMapping mapping;
    REQUIRE(mapping.actionId.empty());
    REQUIRE(mapping.keyCodes.empty());
    REQUIRE(mapping.gamepadButtons.empty());
    REQUIRE(mapping.gamepadAxis == -1);
    REQUIRE(mapping.axisThreshold == Catch::Approx(0.5));

    InputState state;
    REQUIRE(state.mouseX == 0);
    REQUIRE(state.mouseY == 0);
    REQUIRE(state.mouseWheel == 0);
    REQUIRE(state.touchX == 0);
    REQUIRE(state.touchY == 0);
    REQUIRE_FALSE(state.touchPressed);
    REQUIRE(state.touchCount == 0);
    REQUIRE_FALSE(state.gamepadConnected);
    REQUIRE(state.gamepadId == -1);
    REQUIRE(state.dir4 == 0);
    REQUIRE(state.dir8 == 0);
}
