#pragma once

#include "engine/core/battle/battle_formula_probe.h"

namespace urpg::editor {

struct FormulaDebuggerPanelSnapshot {
    size_t probe_count = 0;
    size_t fallback_count = 0;
};

class FormulaDebuggerPanel {
public:
    void loadCases(std::vector<urpg::battle::BattleFormulaProbeCase> cases);
    void render();

    const FormulaDebuggerPanelSnapshot& snapshot() const { return snapshot_; }
    const std::vector<urpg::battle::BattleFormulaProbeResult>& results() const { return results_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    std::vector<urpg::battle::BattleFormulaProbeResult> results_;
    FormulaDebuggerPanelSnapshot snapshot_{};
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
