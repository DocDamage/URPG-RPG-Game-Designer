#include "engine/core/ecs/world.h"
#include "engine/core/ecs/health_system.h"
#include "engine/core/render/camera_shake_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("HealthSystem manages damage and i-frames", "[ecs][health]") {
    urpg::World world;
    urpg::HealthSystem healthSys;

    auto targetId = world.CreateEntity();
    auto& health = world.AddComponent(targetId, urpg::HealthComponent{100, 100});

    SECTION("Applying damage reduces health") {
        healthSys.applyDamage(world, targetId, 20);
        REQUIRE(health.current == 80);
    }

    SECTION("Damage adds invulnerability component") {
        healthSys.applyDamage(world, targetId, 10);
        REQUIRE(world.GetComponent<urpg::InvulnerabilityComponent>(targetId) != nullptr);
    }

    SECTION("Cannot damage invulnerable entity") {
        world.AddComponent(targetId, urpg::InvulnerabilityComponent{urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(1)});
        
        healthSys.applyDamage(world, targetId, 10);
        REQUIRE(health.current == 100);
    }

    SECTION("Death state is correctly set") {
        healthSys.applyDamage(world, targetId, 100);
        REQUIRE(health.current == 0);
        REQUIRE(health.isAlive == false);
    }
}
