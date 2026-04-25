#pragma once

#include <cstdint>
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

class EconomySimulator {
public:
    static EconomyReport run(const EconomyRoute& route);
};

} // namespace urpg::balance
