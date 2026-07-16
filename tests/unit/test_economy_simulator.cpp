#include "engine/core/balance/economy_simulator.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

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

TEST_CASE("Economy scenarios record seed assumptions strategies curves sensitivity and source findings",
          "[balance][economy][scenario][pcq453]") {
    urpg::balance::EconomyScenario scenario;
    scenario.id = "willow.progression";
    scenario.version = "balance.v3";
    scenario.seed = 4242;
    scenario.iterations = 20;
    scenario.route.starting_gold = 10;
    scenario.route.steps = {
        {"quest", 25, 40, {}, false, 0},
        {"walk.1", 0, 0, {}, false, 0},
        {"walk.2", 0, 0, {}, false, 0},
        {"walk.3", 0, 0, {}, false, 0},
        {"buy.key", 0, 0, "item.key", true, 80},
        {"battle", 30, 80, {}, false, 0},
        {"optional", 0, 0, "item.tonic", false, 20},
    };
    scenario.strategies = {
        {"hoarder", 0, 150, 150},
        {"spender", 100, 75, 75},
    };
    scenario.level_xp_curve = {50, 100, 200};
    scenario.sensitivity_starting_gold = 100;
    scenario.max_reward_gap = 2;

    const auto first = urpg::balance::EconomySimulator::runScenario(scenario);
    const auto second = urpg::balance::EconomySimulator::runScenario(scenario);
    REQUIRE(first.scenario_id == "willow.progression");
    REQUIRE(first.scenario_version == "balance.v3");
    REQUIRE(first.seed == 4242);
    REQUIRE(first.iterations == 20);
    REQUIRE(first.assumptions.at("strategy_count") == "2");
    REQUIRE(first.strategies.size() == 2);
    REQUIRE(first.strategies == second.strategies);
    REQUIRE(first.strategies.front().strategy_id == "hoarder");
    REQUIRE(first.strategies.front().average_level >= 3.0);
    REQUIRE(first.sensitivities.size() == 1);
    REQUIRE(first.sensitivities.front().parameter == "starting_gold");
    REQUIRE(first.sensitivities.front().final_gold_delta == 20.0);
    REQUIRE(std::any_of(first.findings.begin(), first.findings.end(), [](const auto& finding) {
        return finding.code == "required_purchase_deadlock" && finding.step_id == "buy.key" &&
               finding.source_route.ends_with("/buy.key/cost");
    }));
    REQUIRE(std::any_of(first.findings.begin(), first.findings.end(), [](const auto& finding) {
        return finding.code == "reward_cadence_gap" && finding.step_id == "walk.3";
    }));
    REQUIRE(std::any_of(first.findings.begin(), first.findings.end(), [](const auto& finding) {
        return finding.code == "dominant_strategy" && finding.strategy_id == "hoarder";
    }));
}

TEST_CASE("Economy scenario validation rejects incomplete reproducibility identity",
          "[balance][economy][scenario][pcq453]") {
    const auto report = urpg::balance::EconomySimulator::runScenario({});
    REQUIRE(report.strategies.empty());
    REQUIRE(report.findings.size() == 1);
    REQUIRE(report.findings.front().code == "scenario_invalid");
}
