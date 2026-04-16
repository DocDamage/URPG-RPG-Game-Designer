#include "engine/core/ecs/world.h"
#include "engine/core/render/camera_shake_system.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CameraShakeSystem applies displacement", "[render][camera]") {
    urpg::World world;
    urpg::CameraShakeSystem shakeSys;

    auto camId = world.CreateEntity();
    world.AddComponent(camId, urpg::TransformComponent{});
    auto& shake = world.AddComponent(camId, urpg::CameraShakeComponent{});
    
    SECTION("Decays trauma over time") {
        shake.trauma = urpg::Fixed32::FromInt(1);
        shake.traumaDecay = urpg::Fixed32::FromInt(1);

        shakeSys.update(world, 0.5f);

        REQUIRE(shake.trauma.ToFloat() == 0.5f);
    }

    SECTION("Trauma results in spatial offset") {
        shake.trauma = urpg::Fixed32::FromInt(1);
        shake.maxTranslation = urpg::Fixed32::FromInt(10);
        
        shakeSys.update(world, 0.1f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(camId);
        // Position should have changed from original zero
        REQUIRE((transform->position.x.ToFloat() != 0.0f || transform->position.y.ToFloat() != 0.0f));
    }

    SECTION("Zero trauma results in no movement") {
        shake.trauma = urpg::Fixed32::FromInt(0);
        
        shakeSys.update(world, 0.1f);

        auto* transform = world.GetComponent<urpg::TransformComponent>(camId);
        REQUIRE(transform->position.x.ToFloat() == 0.0f);
        REQUIRE(transform->position.y.ToFloat() == 0.0f);
    }
}
