#include "engine/core/ecs/world.h"
#include "engine/core/ecs/progression_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ProgressionSystem awards XP and levels up", "[gameplay][progression]") {
    urpg::World world;
    urpg::ProgressionSystem progSys;
    urpg::ActorManager actorMgr(world);

    auto playerId = actorMgr.CreateActor("Hero");
    world.AddComponent(playerId, urpg::PlayerControlComponent{});
    auto& exp = world.AddComponent(playerId, urpg::ExperienceComponent{0, 100, 1, 1.5f});

    auto enemyId = world.CreateEntity();
    auto& health = world.AddComponent(enemyId, urpg::HealthComponent{100, 100, false});
    world.AddComponent(enemyId, urpg::LootComponent{100, 50, {}});

    SECTION("Defeating enemy awards XP") {
        health.isAlive = false; // Mark as dead
        progSys.update(world);

        REQUIRE(exp.level == 2);
        REQUIRE(exp.currentExp == 0);
    }
}
