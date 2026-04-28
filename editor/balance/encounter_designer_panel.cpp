#include "editor/balance/encounter_designer_panel.h"

#include <utility>

namespace urpg::editor::balance {

void EncounterDesignerPanel::loadDocument(urpg::balance::EncounterDesignerDocument document) {
    document_ = std::move(document);
    if (selected_region_id_.empty() && !document_.regions.empty()) {
        selected_region_id_ = document_.regions.front().id;
    }
    refresh();
}

void EncounterDesignerPanel::setPreviewContext(std::string region_id,
                                               int32_t party_level,
                                               std::set<std::string> active_flags,
                                               uint64_t seed,
                                               std::size_t count) {
    selected_region_id_ = std::move(region_id);
    party_level_ = party_level;
    active_flags_ = std::move(active_flags);
    seed_ = seed;
    count_ = count;
    refresh();
}

void EncounterDesignerPanel::render() {
    refresh();
}

nlohmann::json EncounterDesignerPanel::saveProjectData() const {
    return document_.toJson();
}

void EncounterDesignerPanel::refresh() {
    preview_ = document_.preview(selected_region_id_, party_level_, active_flags_, seed_, count_);
    snapshot_.selected_region_id = selected_region_id_;
    snapshot_.party_level = party_level_;
    snapshot_.seed = seed_;
    snapshot_.preview_count = preview_.encounters.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::balance
