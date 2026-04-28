#include "editor/gameplay/gameplay_wysiwyg_panel.h"

#include <utility>

namespace urpg::editor::gameplay {

void GameplayWysiwygPanel::loadDocument(urpg::gameplay::GameplayWysiwygDocument document) {
    document_ = std::move(document);
    if (!document_.rules.empty()) {
        trigger_ = document_.rules.front().trigger;
    }
    refresh();
}

void GameplayWysiwygPanel::setPreviewContext(urpg::gameplay::GameplayWysiwygState state, std::string trigger) {
    state_ = std::move(state);
    trigger_ = std::move(trigger);
    refresh();
}

void GameplayWysiwygPanel::render() {
    refresh();
}

urpg::gameplay::GameplayWysiwygPreview GameplayWysiwygPanel::executePreview() {
    preview_ = document_.execute(state_, trigger_);
    refresh();
    return preview_;
}

nlohmann::json GameplayWysiwygPanel::saveProjectData() const {
    return document_.toJson();
}

void GameplayWysiwygPanel::refresh() {
    preview_ = document_.preview(state_, trigger_);
    snapshot_.feature_id = preview_.feature_id;
    snapshot_.feature_type = preview_.feature_type;
    snapshot_.trigger = trigger_;
    snapshot_.visual_layer_count = document_.visual_layers.size();
    snapshot_.active_rule_count = preview_.active_rules.size();
    snapshot_.event_count = preview_.events.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::gameplay
