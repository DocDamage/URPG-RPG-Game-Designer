#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_follow_system.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/render/camera_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CameraFollowSystem updates camera position", "[render][camera]") {
    urpg::World world;
    urpg::CameraFollowSystem followSys;

    auto targetId = world.CreateEntity();
    world.AddComponent(targetId, urpg::TransformComponent{
        {urpg::Fixed32::FromInt(50), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)},
        {},
        {}
    });

    auto camId = world.CreateEntity();
    world.AddComponent(camId, urpg::TransformComponent{});
    world.AddComponent(camId, urpg::CameraComponent{});
    world.AddComponent(camId, urpg::CameraFollowComponent{targetId, urpg::CameraFollowMode::Simple, {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(-10)}});

    SECTION("Simple follow snaps to target + offset") {
        followSys.update(world, 0.1f);

        auto* camTrans = world.GetComponent<urpg::TransformComponent>(camId);
        REQUIRE(camTrans->position.x == urpg::Fixed32::FromInt(50));
        REQUIRE(camTrans->position.z == urpg::Fixed32::FromInt(-10));
    }

    SECTION("Smooth follow progresses toward target") {
        auto* follow = world.GetComponent<urpg::CameraFollowComponent>(camId);
        follow->mode = urpg::CameraFollowMode::Smooth;
        follow->smoothFactor = urpg::Fixed32::FromRaw(32768); // 0.5 speed

        followSys.update(world, 0.1f); // One step

        auto* camTrans = world.GetComponent<urpg::TransformComponent>(camId);
        // Halfway between 0 and 50 is 25
        REQUIRE(camTrans->position.x == urpg::Fixed32::FromInt(25));
    }
}
