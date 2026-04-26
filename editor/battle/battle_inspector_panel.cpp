#include "editor/battle/battle_inspector_panel.h"

#include <utility>

namespace urpg::editor {

namespace {

urpg::battle::BattleDamageContext DefaultPhysicalPreviewContext() {
    urpg::battle::BattleDamageContext context;
    context.subject.atk = 24;
    context.target.def = 12;
    context.target.hp = 240;
    context.power = 10;
    context.variance_percent = 20;
    return context;
}

urpg::battle::BattleDamageContext DefaultMagicalPreviewContext() {
    urpg::battle::BattleDamageContext context;
    context.subject.mat = 28;
    context.target.mdf = 11;
    context.target.hp = 240;
    context.power = 14;
    context.magical = true;
    context.variance_percent = 10;
    return context;
}

} // namespace

void BattleInspectorPanel::bindRuntime(const urpg::battle::BattleFlowController& flow_controller,
                                       const urpg::battle::BattleActionQueue& action_queue) {
    flow_controller_ = &flow_controller;
    action_queue_ = &action_queue;
    preview_override_.reset();
    preview_panel_.bindRuntime(flow_controller);
}

void BattleInspectorPanel::clearRuntime() {
    flow_controller_ = nullptr;
    action_queue_ = nullptr;
    preview_override_.reset();
    model_ = BattleInspectorModel{};
    preview_panel_.clearRuntime();
    captureRenderSnapshot();
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
    captureRenderSnapshot();
}

void BattleInspectorPanel::refresh() {
    if (!flow_controller_ || !action_queue_) {
        preview_panel_.refresh();
        captureRenderSnapshot();
        return;
    }

    model_.LoadFromRuntime(*flow_controller_, *action_queue_);
    model_.SetShowIssuesOnly(show_issues_only_);
    model_.SetSubjectFilter(subject_filter_);
    applyPreviewBinding();
    preview_panel_.refresh();
    captureRenderSnapshot();
}

void BattleInspectorPanel::update() {
    refresh();
}

const BattleInspectorPanel::RenderSnapshot& BattleInspectorPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

void BattleInspectorPanel::applyDefaultPreviewBinding() {
    preview_panel_.setPhysicalPreviewContext(DefaultPhysicalPreviewContext());
    preview_panel_.setMagicalPreviewContext(DefaultMagicalPreviewContext());
    preview_panel_.setEscapePreviewAgility(100, 100);
}

void BattleInspectorPanel::applyPreviewBinding() {
    if (!preview_override_.has_value()) {
        applyDefaultPreviewBinding();
        return;
    }

    preview_panel_.setPhysicalPreviewContext(preview_override_->physical_preview);
    preview_panel_.setMagicalPreviewContext(preview_override_->magical_preview);
    preview_panel_.setEscapePreviewAgility(preview_override_->party_agi, preview_override_->troop_agi);
}

void BattleInspectorPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = visible_;
    last_render_snapshot_.runtime_bound = flow_controller_ != nullptr && action_queue_ != nullptr;
    last_render_snapshot_.can_refresh = last_render_snapshot_.runtime_bound;
    last_render_snapshot_.visible_row_count = model_.VisibleRows().size();
    last_render_snapshot_.issue_count = model_.Summary().issue_count;
    last_render_snapshot_.phase = model_.Summary().phase;

    if (!last_render_snapshot_.runtime_bound) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "No battle runtime is bound.";
        last_render_snapshot_.remediation =
            "Bind BattleFlowController and BattleActionQueue before rendering battle diagnostics.";
        return;
    }

    if (model_.Summary().issue_count > 0) {
        last_render_snapshot_.status = "error";
        last_render_snapshot_.message = "Battle queue contains diagnostics.";
        last_render_snapshot_.remediation = "Review invalid queued actions before continuing battle simulation.";
        return;
    }

    if (model_.VisibleRows().empty()) {
        last_render_snapshot_.status = "empty";
        last_render_snapshot_.message = "No battle actions are queued.";
        last_render_snapshot_.remediation = "Queue a battle action or advance the battle flow to inspect runtime actions.";
        return;
    }

    last_render_snapshot_.status = "ready";
    last_render_snapshot_.message = "Battle diagnostics are available.";
    last_render_snapshot_.remediation = "";
}

} // namespace urpg::editor
