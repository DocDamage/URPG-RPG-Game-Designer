#include "engine/core/ecs/world.h"
#include "engine/core/ecs/movement_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MovementSystem updates position based on velocity", "[ecs][movement]") {
    urpg::World world;
    urpg::ActorManager actorMgr(world);
    urpg::MovementSystem movementSys;

    SECTION("Static entity does not move") {
        const auto id = actorMgr.CreateActor("Static");
        actorMgr.SetActorPosition(id, urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(10));
        actorMgr.SetActorVelocity(id, urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0));

        movementSys.update(world, 1.0f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(id);
        REQUIRE(transform->position.x == urpg::Fixed32::FromInt(10));
        REQUIRE(transform->position.y == urpg::Fixed32::FromInt(10));
    }

    SECTION("Moving entity updates position") {
        const auto id = actorMgr.CreateActor("Mover");
        actorMgr.SetActorPosition(id, urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0));
        actorMgr.SetActorVelocity(id, urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(-2));

        // Move for 2 seconds
        movementSys.update(world, 2.0f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(id);
        REQUIRE(transform->position.x == urpg::Fixed32::FromInt(10));
        REQUIRE(transform->position.y == urpg::Fixed32::FromInt(-4));
    }

    SECTION("Rotation updates correctly") {
        const auto id = actorMgr.CreateActor("Spinner");
        auto* velocity = world.GetComponent<urpg::VelocityComponent>(id);
        velocity->angular = {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(90)};

        movementSys.update(world, 1.0f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(id);
        REQUIRE(transform->rotation.z == urpg::Fixed32::FromInt(90));
    }
}
