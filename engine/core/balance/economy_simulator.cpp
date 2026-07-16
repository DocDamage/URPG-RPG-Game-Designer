#include "engine/core/balance/economy_simulator.h"

#include <algorithm>
#include <random>

namespace urpg::balance {

EconomyReport EconomySimulator::run(const EconomyRoute& route) {
    EconomyReport report;
    report.final_gold = route.starting_gold;
    for (const auto& step : route.steps) {
        report.final_gold += step.gold_delta;
        report.total_xp += step.xp_delta;
        if (step.cost > 0) {
            if (report.final_gold < step.cost) {
                if (step.required) {
                    report.diagnostics.push_back("unaffordable_required_item:" + step.item_id);
                }
                continue;
            }
            report.final_gold -= step.cost;
        }
    }
    return report;
}

EconomyScenarioReport EconomySimulator::runScenario(const EconomyScenario& scenario) {
    EconomyScenarioReport report;
    report.scenario_id = scenario.id;
    report.scenario_version = scenario.version;
    report.seed = scenario.seed;
    report.iterations = scenario.iterations;
    report.assumptions = {
        {"route_steps", std::to_string(scenario.route.steps.size())},
        {"strategy_count", std::to_string(scenario.strategies.size())},
        {"level_curve_points", std::to_string(scenario.level_xp_curve.size())},
        {"max_reward_gap", std::to_string(scenario.max_reward_gap)},
    };
    if (scenario.id.empty() || scenario.version.empty() || scenario.iterations == 0 || scenario.strategies.empty()) {
        report.findings.push_back({"scenario_invalid", "error", {}, {}, {}, "balance://scenario",
                                   "Scenario requires stable identity, version, iterations, and strategies."});
        return report;
    }

    const auto simulate = [&](const EconomyStrategy& strategy, const int32_t startingGold,
                              const uint64_t seed, const bool collectFindings) {
        EconomyStrategyResult result;
        result.strategy_id = strategy.id;
        std::mt19937_64 rng(seed);
        for (std::size_t iteration = 0; iteration < scenario.iterations; ++iteration) {
            int32_t gold = startingGold;
            int32_t xp = 0;
            std::size_t rewardGap = 0;
            for (const auto& step : scenario.route.steps) {
                const auto goldReward = step.gold_delta * strategy.gold_reward_percent / 100;
                const auto xpReward = step.xp_delta * strategy.xp_reward_percent / 100;
                gold += goldReward;
                xp += xpReward;
                if (goldReward > 0 || xpReward > 0) rewardGap = 0;
                else ++rewardGap;
                if (collectFindings && iteration == 0 && rewardGap > scenario.max_reward_gap) {
                    report.findings.push_back({"reward_cadence_gap", "warning", strategy.id, step.id, "reward",
                        "balance://scenario/" + scenario.id + "/step/" + step.id,
                        "Reward cadence exceeds the configured maximum gap."});
                }
                if (step.cost <= 0) continue;
                const bool wantsPurchase = step.required ||
                    static_cast<int32_t>(rng() % 100) < std::clamp(strategy.optional_purchase_chance, 0, 100);
                if (!wantsPurchase) continue;
                if (gold < step.cost) {
                    if (step.required) {
                        ++result.required_deadlocks;
                        if (collectFindings && iteration == 0) {
                            report.findings.push_back({"required_purchase_deadlock", "error", strategy.id, step.id,
                                "cost", "balance://scenario/" + scenario.id + "/step/" + step.id + "/cost",
                                "Required progression purchase is unaffordable."});
                        }
                    }
                } else gold -= step.cost;
            }
            int32_t level = 1;
            for (const auto threshold : scenario.level_xp_curve) if (xp >= threshold) ++level;
            result.average_final_gold += static_cast<double>(gold);
            result.average_total_xp += static_cast<double>(xp);
            result.average_level += static_cast<double>(level);
        }
        result.average_final_gold /= static_cast<double>(scenario.iterations);
        result.average_total_xp /= static_cast<double>(scenario.iterations);
        result.average_level /= static_cast<double>(scenario.iterations);
        return result;
    };

    for (std::size_t index = 0; index < scenario.strategies.size(); ++index) {
        const auto& strategy = scenario.strategies[index];
        if (strategy.id.empty()) {
            report.findings.push_back({"strategy_id_missing", "error", {}, {}, "id",
                                       "balance://scenario/" + scenario.id + "/strategy",
                                       "Every strategy requires a stable ID."});
            continue;
        }
        report.strategies.push_back(simulate(strategy, scenario.route.starting_gold, scenario.seed + index, true));
    }
    std::sort(report.strategies.begin(), report.strategies.end(), [](const auto& left, const auto& right) {
        const auto leftScore = left.average_final_gold + left.average_total_xp;
        const auto rightScore = right.average_final_gold + right.average_total_xp;
        return leftScore > rightScore;
    });
    if (report.strategies.size() > 1) {
        const auto best = report.strategies[0].average_final_gold + report.strategies[0].average_total_xp;
        const auto next = report.strategies[1].average_final_gold + report.strategies[1].average_total_xp;
        if (best > next * 1.25) {
            report.findings.push_back({"dominant_strategy", "warning", report.strategies[0].strategy_id, {},
                "strategy", "balance://scenario/" + scenario.id + "/strategy/" + report.strategies[0].strategy_id,
                "One strategy outperforms the next alternative by more than 25 percent."});
        }
    }
    if (scenario.sensitivity_starting_gold != 0 && !scenario.strategies.empty()) {
        const auto baseline = simulate(scenario.strategies.front(), scenario.route.starting_gold,
                                       scenario.seed, false).average_final_gold;
        const auto testedStart = scenario.route.starting_gold + scenario.sensitivity_starting_gold;
        const auto tested = simulate(scenario.strategies.front(), testedStart, scenario.seed, false).average_final_gold;
        report.sensitivities.push_back({"starting_gold", scenario.route.starting_gold, testedStart, tested - baseline});
    }
    return report;
}

} // namespace urpg::balance
