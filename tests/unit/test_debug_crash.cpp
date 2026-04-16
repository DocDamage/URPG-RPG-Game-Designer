#include <catch2/catch_test_macros.hpp>
#include "engine/gameplay/combat/combat_calc.h"
#include <string>
#include <unordered_map>

using namespace urpg;

TEST_CASE("Debug Crash: Minimal", "[debug]") {
    std::string s = "Hello";
    std::unordered_map<std::string, float> m;
    m["test"] = 1.0f;
    REQUIRE(s == "Hello");
    REQUIRE(m["test"] == 1.0f);
}

TEST_CASE("Debug Crash: CombatCalc", "[debug][combat]") {
    CombatCalc calc;
    ActorStats attacker;
    attacker.atk = Fixed32::FromInt(50);
    attacker.element = "Fire";
    ActorStats defender;
    defender.def = Fixed32::FromInt(25);

    auto result = calc.CalculateDamage(attacker, defender, 0);
    REQUIRE(result.damage >= 0);
}
