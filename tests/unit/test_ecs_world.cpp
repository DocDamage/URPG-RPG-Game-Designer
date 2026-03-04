#include "engine/core/ecs/world.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("World creates monotonic EntityIDs", "[ecs]") {
    urpg::World world;
    const auto first = world.CreateEntity();
    const auto second = world.CreateEntity();

    REQUIRE(first < second);
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
