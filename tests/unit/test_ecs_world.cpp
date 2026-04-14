#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("World creates monotonic EntityIDs", "[ecs]") {
    urpg::World world;
    const auto first = world.CreateEntity();
    const auto second = world.CreateEntity();

    REQUIRE(first < second);
}

TEST_CASE("ActorManager can create and modify actors", "[ecs][actor]") {
    urpg::World world;
    urpg::ActorManager actorMgr(world);

    const auto heroId = actorMgr.CreateActor("Hero", false);
    const auto goblinId = actorMgr.CreateActor("Goblin", true);

    SECTION("Actors are unique entities") {
        REQUIRE(heroId != goblinId);
    }

    SECTION("Actor properties are preserved") {
        auto* hero = actorMgr.GetActor(heroId);
        REQUIRE(hero != nullptr);
        REQUIRE(hero->name == "Hero");
        REQUIRE(hero->isEnemy == false);

        auto* goblin = actorMgr.GetActor(goblinId);
        REQUIRE(goblin != nullptr);
        REQUIRE(goblin->name == "Goblin");
        REQUIRE(goblin->isEnemy == true);
    }

    SECTION("Actor components can be modified") {
        actorMgr.SetActorPosition(heroId, 10, 20);
        auto* transform = world.GetComponent<urpg::TransformComponent>(heroId);
        REQUIRE(transform != nullptr);
        REQUIRE(transform->x == 10);
        REQUIRE(transform->y == 20);
    }
}

TEST_CASE("ForEachWith iteration is deterministic by EntityID", "[ecs]") {
    urpg::World world;
    const auto first = world.CreateEntity();
    const auto second = world.CreateEntity();

    uint32_t observed_first = 0;
    uint32_t observed_second = 0;

    world.ForEachWith<>([&](urpg::EntityID id) {
        if (observed_first == 0) {
            observed_first = id;
        } else {
            observed_second = id;
        }
    });

    REQUIRE(first < second);
    REQUIRE(observed_first == first);
    REQUIRE(observed_second == second);
}
