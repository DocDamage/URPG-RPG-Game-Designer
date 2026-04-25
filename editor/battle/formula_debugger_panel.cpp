#include "editor/battle/formula_debugger_panel.h"

#include <algorithm>

namespace urpg::editor {

void FormulaDebuggerPanel::loadCases(std::vector<urpg::battle::BattleFormulaProbeCase> cases) {
    results_ = urpg::battle::ProbeBattleFormulas(cases);
    snapshot_.probe_count = results_.size();
    snapshot_.fallback_count = static_cast<size_t>(std::count_if(results_.begin(), results_.end(), [](const auto& result) {
        return result.used_fallback;
    }));
}

void FormulaDebuggerPanel::render() {
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
