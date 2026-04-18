#include "editor/battle/battle_inspector_panel.h"

#include <utility>

namespace urpg::editor {

void BattleInspectorPanel::bindRuntime(const urpg::battle::BattleFlowController& flow_controller,
                                       const urpg::battle::BattleActionQueue& action_queue) {
    flow_controller_ = &flow_controller;
    action_queue_ = &action_queue;
    preview_panel_.bindRuntime(flow_controller);
}

void BattleInspectorPanel::clearRuntime() {
    flow_controller_ = nullptr;
    action_queue_ = nullptr;
    model_ = BattleInspectorModel{};
    preview_panel_.clearRuntime();
}

BattleInspectorModel& BattleInspectorPanel::getModel() {
    return model_;
}

const BattleInspectorModel& BattleInspectorPanel::getModel() const {
    return model_;
}

BattlePreviewPanel& BattleInspectorPanel::previewPanel() {
    return preview_panel_;
}

const BattlePreviewPanel& BattleInspectorPanel::previewPanel() const {
    return preview_panel_;
}

void BattleInspectorPanel::setVisible(bool visible) {
    visible_ = visible;
    preview_panel_.setVisible(visible);
}

bool BattleInspectorPanel::isVisible() const {
    return visible_;
}

void BattleInspectorPanel::setShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    model_.SetShowIssuesOnly(show_issues_only_);
}

void BattleInspectorPanel::setSubjectFilter(std::optional<std::string> subject_filter) {
    subject_filter_ = std::move(subject_filter);
    model_.SetSubjectFilter(subject_filter_);
}

void BattleInspectorPanel::render() {
    if (!visible_) {
        return;
    }
    preview_panel_.render();
}

void BattleInspectorPanel::refresh() {
    if (!flow_controller_ || !action_queue_) {
        preview_panel_.refresh();
        return;
    }

    model_.LoadFromRuntime(*flow_controller_, *action_queue_);
    model_.SetShowIssuesOnly(show_issues_only_);
    model_.SetSubjectFilter(subject_filter_);
    preview_panel_.refresh();
}

void BattleInspectorPanel::update() {
    refresh();
}

} // namespace urpg::editor
