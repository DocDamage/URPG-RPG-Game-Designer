#include "engine/core/input/controller_input_provider.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Controller input provider routes hot-plug buttons axes ownership and aggregate releases",
          "[input][controller][provider][pcq650]") {
    using namespace urpg;
    input::ControllerInputProvider provider;
    input::InputCore core;
    REQUIRE(provider.connect("pad.one", "xbox", accessibility::InputOwnership::PlayerOne, 0.2F, 1.0F));
    REQUIRE(provider.connect("pad.two", "switch", accessibility::InputOwnership::PlayerTwo, 0.25F, 1.0F));
    REQUIRE(provider.connectedDeviceCount() == 2);

    const auto dpad = provider.buttonEvent(core, "pad.one", action::ControllerButton::DPadUp,
                                           input::ActionState::Pressed,
                                           accessibility::InclusiveInputContext::Gameplay);
    REQUIRE(dpad.accepted);
    REQUIRE(dpad.action == input::InputAction::MoveUp);
    REQUIRE(dpad.glyph == "xbox:DPadUp");
    REQUIRE(core.isActionJustPressed(input::InputAction::MoveUp));
    core.endFrame();

    const auto axis = provider.axisEvent(core, "pad.two", input::ControllerAxis::LeftY, -0.8F,
                                         accessibility::InclusiveInputContext::Gameplay);
    REQUIRE(axis.accepted);
    REQUIRE(axis.action == input::InputAction::MoveUp);
    REQUIRE(axis.normalized_value < 0.0F);
    REQUIRE(provider.activeDeviceId() == "pad.two");
    REQUIRE(core.isActionActive(input::InputAction::MoveUp));

    REQUIRE(provider.disconnect(core, "pad.one"));
    REQUIRE(provider.connectedDeviceCount() == 1);
    REQUIRE(core.isActionActive(input::InputAction::MoveUp));
    const auto neutral = provider.axisEvent(core, "pad.two", input::ControllerAxis::LeftY, 0.0F,
                                            accessibility::InclusiveInputContext::Gameplay);
    REQUIRE(neutral.accepted);
    REQUIRE(core.isActionJustReleased(input::InputAction::MoveUp));

    REQUIRE(provider.reconnect("pad.one"));
    REQUIRE(provider.calibrate("pad.one", 0.4F, 1.2F));
    const auto deadzone = provider.axisEvent(core, "pad.one", input::ControllerAxis::LeftX, 0.3F,
                                             accessibility::InclusiveInputContext::Gameplay);
    REQUIRE_FALSE(deadzone.accepted);
    REQUIRE(provider.recoveryActions() ==
            std::vector<std::string>{"reset-bindings", "open-calibration", "continue-controller"});

    action::ControllerBindingRuntime custom;
    custom.bindButton(action::ControllerButton::FaceBottom, input::InputAction::Cancel);
    custom.bindButton(action::ControllerButton::FaceRight, input::InputAction::Confirm);
    REQUIRE(provider.applyBindings(custom));
    const auto rebound = provider.buttonEvent(core, "pad.one", action::ControllerButton::FaceBottom,
                                               input::ActionState::Pressed,
                                               accessibility::InclusiveInputContext::Menu);
    REQUIRE(rebound.accepted);
    REQUIRE(rebound.action == input::InputAction::Cancel);
}
