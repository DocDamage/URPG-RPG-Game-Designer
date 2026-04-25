#include "editor/spatial/region_rules_panel.h"

#include <utility>

namespace urpg::editor {

void RegionRulesPanel::loadRules(std::vector<urpg::map::MapRegionRule> rules) {
    rules_ = std::move(rules);
    diagnostics_ = urpg::map::ValidateMapRegionRules(rules_);
    snapshot_ = {rules_.size(), diagnostics_.size()};
}

void RegionRulesPanel::render() {}

} // namespace urpg::editor
