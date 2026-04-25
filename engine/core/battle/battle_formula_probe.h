#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::battle {

struct BattleFormulaProbeCase {
    std::string id;
    std::string formula;
};

struct BattleFormulaProbeResult {
    std::string id;
    std::string formula;
    int32_t value = 0;
    bool used_fallback = false;
    std::string reason;
    std::string normalized_formula;
};

std::vector<BattleFormulaProbeResult> ProbeBattleFormulas(const std::vector<BattleFormulaProbeCase>& cases);

} // namespace urpg::battle
