#pragma once

#include "engine/core/map/map_region_rules.h"

#include <cstddef>
#include <vector>

namespace urpg::editor {

struct RegionRulesPanelSnapshot {
    size_t rule_count = 0;
    size_t diagnostic_count = 0;
};

class RegionRulesPanel {
public:
    void loadRules(std::vector<urpg::map::MapRegionRule> rules);
    void render();

    const RegionRulesPanelSnapshot& snapshot() const { return snapshot_; }
    const std::vector<urpg::map::MapDiagnostic>& diagnostics() const { return diagnostics_; }

private:
    std::vector<urpg::map::MapRegionRule> rules_;
    std::vector<urpg::map::MapDiagnostic> diagnostics_;
    RegionRulesPanelSnapshot snapshot_{};
};

} // namespace urpg::editor
