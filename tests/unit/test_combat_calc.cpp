#include "engine/gameplay/combat/combat_calc.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Physical damage formula baseline", "[combat]") {
    urpg::CombatCalc calc;

    urpg::ActorStats attacker{
        .level = 10,
        .atk = urpg::Fixed32::FromInt(100),
        .def = urpg::Fixed32::FromInt(0)
    };

    urpg::ActorStats defender{
        .level = 10,
        .atk = urpg::Fixed32::FromInt(0),
        .def = urpg::Fixed32::FromInt(50)
    };

    const auto result = calc.PhysicalDamage(attacker, defender, 0);

    REQUIRE(result.damage == 50);
    REQUIRE_FALSE(result.critical);
}
