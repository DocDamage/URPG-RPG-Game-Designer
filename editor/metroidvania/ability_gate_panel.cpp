#include "editor/metroidvania/ability_gate_panel.h"

#include <utility>

namespace urpg::editor::metroidvania {

void AbilityGatePanel::loadDocument(urpg::metroidvania::AbilityGateDocument document) {
    document_ = std::move(document);
    if (start_region_.empty() && !document_.regions.empty()) {
        start_region_ = document_.regions.front().id;
    }
    refresh();
}

void AbilityGatePanel::setPreviewContext(std::string start_region, std::set<std::string> initial_abilities) {
    start_region_ = std::move(start_region);
    initial_abilities_ = std::move(initial_abilities);
    refresh();
}

void AbilityGatePanel::render() {
    refresh();
}

nlohmann::json AbilityGatePanel::saveProjectData() const {
    return document_.toJson();
}

void AbilityGatePanel::refresh() {
    preview_ = document_.preview(start_region_, initial_abilities_);
    snapshot_.start_region = start_region_;
    snapshot_.reachable_count = preview_.reachable_regions.size();
    snapshot_.unlocked_ability_count = preview_.unlocked_abilities.size();
    snapshot_.blocked_link_count = preview_.blocked_links.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::metroidvania
