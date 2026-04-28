#include "editor/maker/maker_wysiwyg_panel.h"

#include <utility>

namespace urpg::editor::maker {

void MakerWysiwygPanel::loadDocument(urpg::maker::MakerWysiwygFeatureDocument document) {
    document_ = std::move(document);
    if (!document_.actions.empty()) {
        trigger_ = document_.actions.front().trigger;
    }
    refresh();
}

void MakerWysiwygPanel::setPreviewContext(urpg::maker::MakerFeatureRuntimeState state, std::string trigger) {
    state_ = std::move(state);
    trigger_ = std::move(trigger);
    refresh();
}

void MakerWysiwygPanel::render() {
    refresh();
}

urpg::maker::MakerFeaturePreview MakerWysiwygPanel::executePreview() {
    preview_ = document_.execute(state_, trigger_);
    refresh();
    return preview_;
}

nlohmann::json MakerWysiwygPanel::saveProjectData() const {
    return document_.toJson();
}

void MakerWysiwygPanel::refresh() {
    preview_ = document_.preview(state_, trigger_);
    snapshot_.feature_type = preview_.feature_type;
    snapshot_.trigger = trigger_;
    snapshot_.visual_layer_count = document_.visual_layers.size();
    snapshot_.active_action_count = preview_.active_actions.size();
    snapshot_.emitted_operation_count = preview_.resulting_state.emitted_operations.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::maker
