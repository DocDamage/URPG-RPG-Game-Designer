#include "engine/core/ecs/actor_manager.h"
#include "engine/core/ecs/system_orchestrator.h"
#include "engine/core/ecs/world.h"
#include "engine/core/input/input_core.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("SystemOrchestrator runs full frame pipeline", "[ecs][integration]") {
    urpg::World world;
    urpg::ActorManager actorMgr(world);
    urpg::input::InputCore input;
    urpg::SystemOrchestrator orchestrator;

    const auto player = actorMgr.CreateActor("Hero");
    world.AddComponent(player, urpg::PlayerControlComponent{urpg::Fixed32::FromInt(10)});

    // Setup a wall
    const auto wall = actorMgr.CreateActor("Wall");
    actorMgr.SetActorPosition(wall, urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(0));
    world.AddComponent(wall, urpg::CollisionBoxComponent{
                                 urpg::Vector3::Zero(),
                                 {urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2)}});

    // Setup player collision
    world.AddComponent(player, urpg::CollisionBoxComponent{
                                   urpg::Vector3::Zero(),
                                   {urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1)}});

    SECTION("Player moves when input is active") {
        input.updateActionState(urpg::input::InputAction::MoveRight, urpg::input::ActionState::Pressed);

        // Run one frame
        orchestrator.update(world, input, 0.1f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(player);
        REQUIRE(transform->position.x.ToFloat() > 0.0f);
    }

    SECTION("Collision stops movement in the same frame") {
        // Teleport player close to wall
        actorMgr.SetActorPosition(player, urpg::Fixed32::FromInt(4), urpg::Fixed32::FromInt(0));
        input.updateActionState(urpg::input::InputAction::MoveRight, urpg::input::ActionState::Pressed);

        orchestrator.update(world, input, 0.1f);

        auto* velocity = world.GetComponent<urpg::VelocityComponent>(player);
        // Collision system should have zeroed the velocity before MovementSystem ran
        REQUIRE(velocity->linear.x == urpg::Fixed32::FromInt(0));
    }
}
