#include "engine/core/battle/battle_formula_probe.h"

#include "engine/core/scene/combat_formula.h"

#include <algorithm>

namespace urpg::battle {

std::vector<BattleFormulaProbeResult> ProbeBattleFormulas(const std::vector<BattleFormulaProbeCase>& cases) {
    std::vector<BattleFormulaProbeCase> ordered = cases;
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.id < b.id;
    });

    std::vector<BattleFormulaProbeResult> results;
    results.reserve(ordered.size());
    const urpg::combat::CombatFormula::Context context{nullptr, nullptr, nullptr, nullptr};
    for (const auto& probe : ordered) {
        const auto evaluation = urpg::combat::CombatFormula::evaluateFormula(probe.formula, context);
        results.push_back({
            probe.id,
            probe.formula,
            evaluation.value,
            evaluation.usedFallback,
            evaluation.reason,
            evaluation.normalizedFormula,
        });
    }
    return results;
}

} // namespace urpg::battle
