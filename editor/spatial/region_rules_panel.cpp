#include "editor/spatial/region_rules_panel.h"

#include <utility>

namespace urpg::editor {

void RegionRulesPanel::loadRules(std::vector<urpg::map::MapRegionRule> rules) {
    rules_ = std::move(rules);
    diagnostics_ = urpg::map::ValidateMapRegionRules(rules_);
    snapshot_ = {
        true,
        snapshot_.rendered,
        false,
        rules_.size(),
        diagnostics_.size(),
        diagnostics_.empty() ? "Region rules are valid." : "Region rules have diagnostics.",
    };
}

void RegionRulesPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    snapshot_.rule_count = rules_.size();
    snapshot_.diagnostic_count = diagnostics_.size();
    if (rules_.empty()) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load region rules before previewing this panel.";
        return;
    }

    snapshot_.disabled = false;
    snapshot_.status_message = diagnostics_.empty() ? "Region rules are valid." : "Region rules have diagnostics.";
}

} // namespace urpg::editor
