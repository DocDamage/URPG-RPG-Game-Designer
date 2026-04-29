#include "editor/battle/battle_preview_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

namespace {

std::string PhaseLabel(urpg::battle::BattleFlowPhase phase) {
    switch (phase) {
    case urpg::battle::BattleFlowPhase::None:
        return "none";
    case urpg::battle::BattleFlowPhase::Start:
        return "start";
    case urpg::battle::BattleFlowPhase::Input:
        return "input";
    case urpg::battle::BattleFlowPhase::Action:
        return "action";
    case urpg::battle::BattleFlowPhase::TurnEnd:
        return "turn_end";
    case urpg::battle::BattleFlowPhase::Victory:
        return "victory";
    case urpg::battle::BattleFlowPhase::Defeat:
        return "defeat";
    case urpg::battle::BattleFlowPhase::Abort:
        return "abort";
    }
    return "none";
}

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

BattlePreviewPanel::BattlePreviewPanel()
    : physical_preview_context_(DefaultPhysicalPreviewContext()),
      magical_preview_context_(DefaultMagicalPreviewContext()) {
    resetSnapshot();
}

void BattlePreviewPanel::bindRuntime(const urpg::battle::BattleFlowController& flow_controller) {
    flow_controller_ = &flow_controller;
}

void BattlePreviewPanel::clearRuntime() {
    flow_controller_ = nullptr;
    resetSnapshot();
}

void BattlePreviewPanel::setPhysicalPreviewContext(const urpg::battle::BattleDamageContext& context) {
    physical_preview_context_ = context;
}

void BattlePreviewPanel::setMagicalPreviewContext(const urpg::battle::BattleDamageContext& context) {
    magical_preview_context_ = context;
}

void BattlePreviewPanel::setFeedbackPolicy(const urpg::battle::BattleFeedbackPolicy& policy) {
    feedback_policy_ = policy;
}

void BattlePreviewPanel::setTroopPositionPreview(
    std::vector<urpg::battle::TroopMemberPosition> authored_positions,
    std::vector<urpg::battle::TroopMemberPosition> reusable_positions) {
    authored_troop_positions_ = std::move(authored_positions);
    reusable_troop_positions_ = std::move(reusable_positions);
}

void BattlePreviewPanel::setEscapePreviewAgility(int32_t party_agi, int32_t troop_agi) {
    preview_party_agi_ = party_agi;
    preview_troop_agi_ = troop_agi;
}

const BattlePreviewSnapshot& BattlePreviewPanel::snapshot() const {
    return snapshot_;
}

const std::vector<BattlePreviewIssue>& BattlePreviewPanel::issues() const {
    return issues_;
}

void BattlePreviewPanel::setVisible(bool visible) {
    visible_ = visible;
}

bool BattlePreviewPanel::isVisible() const {
    return visible_;
}

void BattlePreviewPanel::render() {
    if (!visible_) {
        return;
    }
}

void BattlePreviewPanel::refresh() {
    resetSnapshot();
    if (!flow_controller_) {
        return;
    }

    snapshot_.phase = PhaseLabel(flow_controller_->phase());
    snapshot_.can_escape = flow_controller_->canEscape();

    const auto addIssue = [&](BattlePreviewIssueSeverity severity, std::string code, std::string message) {
        BattlePreviewIssue issue;
        issue.severity = severity;
        issue.code = std::move(code);
        issue.message = std::move(message);
        issues_.push_back(std::move(issue));
    };

    auto physical = physical_preview_context_;
    physical.magical = false;
    auto guarded = physical;
    guarded.target.guarding = true;
    auto critical = physical;
    critical.critical = true;
    auto magical = magical_preview_context_;
    magical.magical = true;

    snapshot_.physical_damage = urpg::battle::BattleRuleResolver::resolveDamage(physical);
    snapshot_.guarded_damage = urpg::battle::BattleRuleResolver::resolveDamage(guarded);
    snapshot_.critical_damage = urpg::battle::BattleRuleResolver::resolveDamage(critical);
    snapshot_.magical_damage = urpg::battle::BattleRuleResolver::resolveDamage(magical);
    const auto feedback = urpg::battle::BattleRuleResolver::resolveFeedbackPreview(
        snapshot_.physical_damage, snapshot_.magical_damage, 1, 2, feedback_policy_);
    snapshot_.chip_damage = feedback.chip_damage;
    snapshot_.chip_healing = feedback.chip_healing;
    snapshot_.buff_level_preview = feedback.buff_level;
    snapshot_.zero_damage_label = feedback.zero_damage_label;
    snapshot_.reused_troop_position_count = urpg::battle::BattleRuleResolver::resolveTroopPositions(
                                                authored_troop_positions_, reusable_troop_positions_, feedback_policy_)
                                                .reused_count;
    snapshot_.escape_ratio_now = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview_party_agi_, preview_troop_agi_, flow_controller_->escapeFailures());
    snapshot_.escape_ratio_next_fail = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview_party_agi_, preview_troop_agi_, flow_controller_->escapeFailures() + 1);

    if (preview_party_agi_ <= 0 || preview_troop_agi_ <= 0) {
        addIssue(BattlePreviewIssueSeverity::Warning, "escape_agi_clamped",
                 "Escape preview agility inputs were clamped to positive range.");
    }
    if (!flow_controller_->canEscape() && flow_controller_->escapeFailures() > 0) {
        addIssue(BattlePreviewIssueSeverity::Warning, "escape_policy_locked",
                 "Escape failures exist while escape is currently unavailable.");
    }
    if (snapshot_.guarded_damage > snapshot_.physical_damage) {
        addIssue(BattlePreviewIssueSeverity::Error, "guard_regression",
                 "Guarded damage preview exceeded baseline physical damage.");
    }
    if (snapshot_.critical_damage < snapshot_.physical_damage) {
        addIssue(BattlePreviewIssueSeverity::Error, "critical_regression",
                 "Critical preview damage should not be lower than baseline.");
    }

    snapshot_.issue_count = issues_.size();
}

void BattlePreviewPanel::update() {
    refresh();
}

void BattlePreviewPanel::resetSnapshot() {
    snapshot_ = {};
    issues_.clear();
}

} // namespace urpg::editor
