#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_system.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CameraSystem identifies active camera", "[render][camera]") {
    urpg::World world;
    urpg::CameraSystem cameraSys;

    SECTION("Finds the first active camera") {
        auto camId = world.CreateEntity();
        world.AddComponent(camId, urpg::TransformComponent{
            {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(-10)}
        });
        world.AddComponent(camId, urpg::CameraComponent{urpg::Fixed32::FromInt(90), urpg::Fixed32::FromInt(1), urpg::Fixed32::FromRaw(65), urpg::Fixed32::FromInt(100), true});

        cameraSys.update(world);

        REQUIRE(cameraSys.hasActiveCamera() == true);
        REQUIRE(cameraSys.getViewPosition().y == urpg::Fixed32::FromInt(5));
        REQUIRE(cameraSys.getCameraData().fov == urpg::Fixed32::FromInt(90));
    }

    SECTION("Ignores inactive cameras") {
        auto camId = world.CreateEntity();
        world.AddComponent(camId, urpg::TransformComponent{});
        world.AddComponent(camId, urpg::CameraComponent{urpg::Fixed32::FromInt(60), urpg::Fixed32::FromInt(1), urpg::Fixed32::FromRaw(65), urpg::Fixed32::FromInt(100), false});

        cameraSys.update(world);

        REQUIRE(cameraSys.hasActiveCamera() == false);
    }
}
