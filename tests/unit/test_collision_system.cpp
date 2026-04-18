#include "engine/core/ecs/world.h"
#include "engine/core/ecs/collision_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CollisionSystem detects overlapping AABBs", "[ecs][collision]") {
    urpg::World world;
    urpg::ActorManager actorMgr(world);
    urpg::CollisionSystem collisionSys;

    SECTION("Overlapping entities stop moving") {
        const auto id1 = actorMgr.CreateActor("Static");
        actorMgr.SetActorPosition(id1, urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0));
        world.AddComponent(id1, urpg::CollisionBoxComponent{
            urpg::Vector3::Zero(),
            {urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2)}
        });

        const auto id2 = actorMgr.CreateActor("Mover");
        actorMgr.SetActorPosition(id2, urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1));
        actorMgr.SetActorVelocity(id2, urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(5));
        world.AddComponent(id2, urpg::CollisionBoxComponent{
            urpg::Vector3::Zero(),
            {urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(2)}
        });

        collisionSys.update(world);

        auto* velocity2 = world.GetComponent<urpg::VelocityComponent>(id2);
        REQUIRE(velocity2->linear.x == urpg::Fixed32::FromInt(0));
        REQUIRE(velocity2->linear.y == urpg::Fixed32::FromInt(0));
    }

    SECTION("Non-overlapping entities keep moving") {
        const auto id1 = actorMgr.CreateActor("Entity1");
        actorMgr.SetActorPosition(id1, urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0));
        world.AddComponent(id1, urpg::CollisionBoxComponent{
            urpg::Vector3::Zero(),
            {urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1)}
        });

        const auto id2 = actorMgr.CreateActor("Entity2");
        actorMgr.SetActorPosition(id2, urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(10));
        actorMgr.SetActorVelocity(id2, urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(5));
        world.AddComponent(id2, urpg::CollisionBoxComponent{
            urpg::Vector3::Zero(),
            {urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1)}
        });

        collisionSys.update(world);

        auto* velocity2 = world.GetComponent<urpg::VelocityComponent>(id2);
        REQUIRE(velocity2->linear.x == urpg::Fixed32::FromInt(5));
        REQUIRE(velocity2->linear.y == urpg::Fixed32::FromInt(5));
    }
}
