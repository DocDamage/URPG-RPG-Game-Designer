#include <catch2/catch_test_macros.hpp>
#include "ability/pattern_ability.h"
#include "ability/ability_system_component.h"
#include <memory>

using namespace urpg::ability;

TEST_CASE("PatternAbility integrates with PatternField", "[ability][pattern]") {
    auto pattern = std::make_shared<PatternField>();
    pattern->name = "Cross";
    pattern->points = {{0,0}, {1,0}, {-1,0}, {0,1}, {0,-1}};

    PatternAbility ability("CrossStrike", pattern);
    AbilitySystemComponent owner;

    SECTION("Can calculate global affected cells") {
        auto affected = ability.getAffectedCells(10, 10);
        
        REQUIRE(affected.size() == 5);
        
        bool foundCenter = false;
        bool foundNorth = false;
        for (const auto& p : affected) {
            if (p.x == 10 && p.y == 10) foundCenter = true;
            if (p.x == 10 && p.y == 11) foundNorth = true;
        }
        
        REQUIRE(foundCenter);
        REQUIRE(foundNorth);
    }

    SECTION("Fails activation without pattern") {
        PatternAbility nullAbility("Broken", nullptr);
        REQUIRE_FALSE(nullAbility.tryActivate(owner));
    }

    SECTION("Successfully activates with pattern") {
        REQUIRE(ability.tryActivate(owner));
    }
}
