#include "engine/core/ecs/world.h"
#include "engine/gameplay/status_effect_system.h"
#include "engine/core/ecs/health_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("StatusEffectSystem applies DOT", "[gameplay][status]") {
    urpg::World world;
    urpg::StatusEffectSystem statusSys;

    auto targetId = world.CreateEntity();
    auto& health = world.AddComponent(targetId, urpg::HealthComponent{100, 100});
    world.AddComponent(targetId, urpg::StatusEffectComponent{
        urpg::StatusEffectComponent::EffectType::Poison,
        urpg::Fixed32::FromInt(5),
        urpg::Fixed32::FromInt(1),
        urpg::Fixed32::FromInt(0),
        10
    });

    SECTION("Poison ticks damage") {
        statusSys.update(world, 1.0f);
        REQUIRE(health.current == 90);
    }
}
