#include "editor/community/community_wysiwyg_panel.h"

#include <utility>

namespace urpg::editor::community {

void CommunityWysiwygPanel::loadDocument(urpg::community::CommunityWysiwygFeatureDocument document) {
    document_ = std::move(document);
    if (!document_.actions.empty()) {
        trigger_ = document_.actions.front().trigger;
    }
    refresh();
}

void CommunityWysiwygPanel::setPreviewContext(urpg::community::CommunityFeatureRuntimeState state, std::string trigger) {
    state_ = std::move(state);
    trigger_ = std::move(trigger);
    refresh();
}

void CommunityWysiwygPanel::render() {
    refresh();
}

urpg::community::CommunityFeaturePreview CommunityWysiwygPanel::executePreview() {
    preview_ = document_.execute(state_, trigger_);
    refresh();
    return preview_;
}

nlohmann::json CommunityWysiwygPanel::saveProjectData() const {
    return document_.toJson();
}

void CommunityWysiwygPanel::refresh() {
    preview_ = document_.preview(state_, trigger_);
    snapshot_.feature_type = preview_.feature_type;
    snapshot_.trigger = trigger_;
    snapshot_.visual_layer_count = document_.visual_layers.size();
    snapshot_.active_action_count = preview_.active_actions.size();
    snapshot_.emitted_command_count = preview_.resulting_state.emitted_commands.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::community
