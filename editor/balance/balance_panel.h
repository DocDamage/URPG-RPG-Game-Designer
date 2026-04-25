#pragma once

#include "engine/core/balance/economy_simulator.h"

namespace urpg::editor {

struct BalancePanelSnapshot {
    int32_t final_gold = 0;
    int32_t total_xp = 0;
    std::size_t diagnostic_count = 0;
};

class BalancePanel {
public:
    void setRoute(balance::EconomyRoute route);
    BalancePanelSnapshot snapshot() const;
    void render();
    const BalancePanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    balance::EconomyRoute route_;
    BalancePanelSnapshot last_render_snapshot_{};
};

} // namespace urpg::editor
