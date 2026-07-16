#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::balance {

struct EconomyStep {
    std::string id;
    int32_t gold_delta = 0;
    int32_t xp_delta = 0;
    std::string item_id;
    bool required = false;
    int32_t cost = 0;
};

struct EconomyRoute {
    int32_t starting_gold = 0;
    std::vector<EconomyStep> steps;
};

struct EconomyReport {
    int32_t final_gold = 0;
    int32_t total_xp = 0;
    std::vector<std::string> diagnostics;
};

struct EconomyStrategy {
    std::string id;
    int32_t optional_purchase_chance = 100;
    int32_t gold_reward_percent = 100;
    int32_t xp_reward_percent = 100;
};

struct EconomyScenario {
    std::string id;
    std::string version;
    EconomyRoute route;
    std::vector<EconomyStrategy> strategies;
    std::vector<int32_t> level_xp_curve;
    uint64_t seed = 0;
    std::size_t iterations = 1;
    int32_t sensitivity_starting_gold = 0;
    std::size_t max_reward_gap = 5;
};

struct EconomyStrategyResult {
    std::string strategy_id;
    double average_final_gold = 0.0;
    double average_total_xp = 0.0;
    double average_level = 0.0;
    std::size_t required_deadlocks = 0;

    bool operator==(const EconomyStrategyResult&) const = default;
};

struct EconomySensitivityResult {
    std::string parameter;
    int32_t baseline = 0;
    int32_t tested = 0;
    double final_gold_delta = 0.0;

    bool operator==(const EconomySensitivityResult&) const = default;
};

struct EconomySimulationFinding {
    std::string code;
    std::string severity;
    std::string strategy_id;
    std::string step_id;
    std::string parameter;
    std::string source_route;
    std::string message;
};

struct EconomyScenarioReport {
    std::string scenario_id;
    std::string scenario_version;
    uint64_t seed = 0;
    std::size_t iterations = 0;
    std::map<std::string, std::string> assumptions;
    std::vector<EconomyStrategyResult> strategies;
    std::vector<EconomySensitivityResult> sensitivities;
    std::vector<EconomySimulationFinding> findings;
};

class EconomySimulator {
public:
    static EconomyReport run(const EconomyRoute& route);
    static EconomyScenarioReport runScenario(const EconomyScenario& scenario);
};

} // namespace urpg::balance
