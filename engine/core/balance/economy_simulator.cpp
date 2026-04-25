#include "engine/core/balance/economy_simulator.h"

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

} // namespace urpg::balance
