#include "engine/core/ecs/world.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/input/input_core.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PlayerControlSystem sets velocity based on input", "[ecs][input]") {
    urpg::World world;
    urpg::ActorManager actorMgr(world);
    urpg::input::InputCore input;
    urpg::PlayerControlSystem controlSys;

    const auto id = actorMgr.CreateActor("Player");
    world.AddComponent(id, urpg::PlayerControlComponent{urpg::Fixed32::FromInt(10)});

    SECTION("No input results in zero velocity") {
        controlSys.update(world, input);
        auto* velocity = world.GetComponent<urpg::VelocityComponent>(id);
        REQUIRE(velocity->linear.x == urpg::Fixed32::FromInt(0));
        REQUIRE(velocity->linear.y == urpg::Fixed32::FromInt(0));
    }

    SECTION("Up input sets negative Y velocity") {
        input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Pressed);
        controlSys.update(world, input);
        auto* velocity = world.GetComponent<urpg::VelocityComponent>(id);
        REQUIRE(velocity->linear.y == urpg::Fixed32::FromInt(-10));
        REQUIRE(velocity->linear.x == urpg::Fixed32::FromInt(0));
    }

    SECTION("Right input sets positive X velocity") {
        input.updateActionState(urpg::input::InputAction::MoveRight, urpg::input::ActionState::Pressed);
        controlSys.update(world, input);
        auto* velocity = world.GetComponent<urpg::VelocityComponent>(id);
        REQUIRE(velocity->linear.x == urpg::Fixed32::FromInt(10));
        REQUIRE(velocity->linear.y == urpg::Fixed32::FromInt(0));
    }

    SECTION("Opposing inputs cancel out") {
        input.updateActionState(urpg::input::InputAction::MoveLeft, urpg::input::ActionState::Pressed);
        input.updateActionState(urpg::input::InputAction::MoveRight, urpg::input::ActionState::Pressed);
        controlSys.update(world, input);
        auto* velocity = world.GetComponent<urpg::VelocityComponent>(id);
        REQUIRE(velocity->linear.x == urpg::Fixed32::FromInt(0));
    }
}
