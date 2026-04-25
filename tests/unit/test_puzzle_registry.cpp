#include "engine/core/puzzle/puzzle_registry.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Puzzle ordered trigger rejects wrong sequence and can reset", "[puzzle][simulation][ffs13]") {
    urpg::puzzle::PuzzleRegistry registry;
    registry.addPuzzle({"puzzle.door", {"red", "blue"}, true, "door_open"});

    REQUIRE_FALSE(registry.trigger("puzzle.door", "blue").solved);
    REQUIRE(registry.state("puzzle.door").failed);

    registry.reset("puzzle.door");
    REQUIRE_FALSE(registry.state("puzzle.door").failed);
    REQUIRE_FALSE(registry.trigger("puzzle.door", "red").solved);
    REQUIRE(registry.trigger("puzzle.door", "blue").solved);
}
