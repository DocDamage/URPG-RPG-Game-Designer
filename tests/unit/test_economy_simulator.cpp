#include "engine/core/balance/economy_simulator.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Economy simulator flags unaffordable required purchases and tracks resources", "[balance][economy][ffs12]") {
    urpg::balance::EconomyRoute route;
    route.starting_gold = 25;
    route.steps = {
        {"quest_reward", 20, 10, {}, false},
        {"buy_key", 0, 0, "item.key", true, 80},
        {"inn", 0, 0, {}, false, 10},
    };

    const auto report = urpg::balance::EconomySimulator::run(route);

    REQUIRE(report.final_gold == 35);
    REQUIRE(report.total_xp == 10);
    REQUIRE(report.diagnostics.size() == 1);
    REQUIRE(report.diagnostics[0] == "unaffordable_required_item:item.key");
}
