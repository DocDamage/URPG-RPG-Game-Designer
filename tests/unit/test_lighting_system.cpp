#include "engine/core/ecs/world.h"
#include "engine/core/render/lighting_system.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("LightingSystem gathers lights from world", "[render][lighting]") {
    urpg::World world;
    urpg::LightingSystem lightSys;

    SECTION("Gathers point lights with correct positions") {
        auto lightId = world.CreateEntity();
        world.AddComponent(lightId, urpg::TransformComponent{
            {urpg::Fixed32::FromInt(5), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)}
        });
        world.AddComponent(lightId, urpg::PointLightComponent{
            {urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)},
            urpg::Fixed32::FromInt(1),
            urpg::Fixed32::FromInt(10)
        });

        lightSys.update(world);

        const auto& lights = lightSys.getVisibleLights();
        REQUIRE(lights.size() == 1);
        REQUIRE(lights[0].position.x == urpg::Fixed32::FromInt(5));
        REQUIRE(lights[0].color.x == urpg::Fixed32::FromInt(1));
    }

    SECTION("Gathers ambient light settings") {
        auto ambientId = world.CreateEntity();
        world.AddComponent(ambientId, urpg::AmbientLightComponent{
            {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(1)},
            urpg::Fixed32::FromInt(2)
        });

        lightSys.update(world);

        REQUIRE(lightSys.getAmbientColor().z == urpg::Fixed32::FromInt(1));
        REQUIRE(lightSys.getAmbientIntensity() == urpg::Fixed32::FromInt(2));
    }
}
