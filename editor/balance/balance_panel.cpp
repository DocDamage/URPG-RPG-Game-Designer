#include "editor/balance/balance_panel.h"

#include <utility>

namespace urpg::editor {

void BalancePanel::setRoute(balance::EconomyRoute route) {
    route_ = std::move(route);
}

BalancePanelSnapshot BalancePanel::snapshot() const {
    const auto report = balance::EconomySimulator::run(route_);
    return BalancePanelSnapshot{report.final_gold, report.total_xp, report.diagnostics.size()};
}

void BalancePanel::render() {
    last_render_snapshot_ = snapshot();
}

} // namespace urpg::editor
