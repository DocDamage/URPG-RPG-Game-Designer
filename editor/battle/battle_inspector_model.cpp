#include "editor/battle/battle_inspector_model.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

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

std::string ActionSummary(const BattleInspectorActionRow& row) {
    std::ostringstream oss;
    oss << (row.subject_id.empty() ? "<missing-subject>" : row.subject_id) << " -> "
        << (row.target_id.empty() ? "<missing-target>" : row.target_id) << " : "
        << (row.command.empty() ? "<missing-command>" : row.command) << " [spd=" << row.speed
        << ", prio=" << row.priority << "]";
    return oss.str();
}

} // namespace

void BattleInspectorModel::LoadFromRuntime(const urpg::battle::BattleFlowController& flow_controller,
                                           const urpg::battle::BattleActionQueue& action_queue) {
    all_rows_.clear();
    visible_rows_.clear();
    issues_.clear();
    selected_row_index_.reset();
    summary_ = {};

    summary_.phase = PhaseLabel(flow_controller.phase());
    summary_.active = flow_controller.isActive();
    summary_.can_escape = flow_controller.canEscape();
    summary_.turn_count = flow_controller.turnCount();
    summary_.escape_failures = flow_controller.escapeFailures();

    const auto ordered_actions = action_queue.snapshotOrdered();
    summary_.total_actions = ordered_actions.size();
    std::vector<size_t> issue_count_by_row(ordered_actions.size(), 0);
    std::unordered_set<std::string> unique_subjects;
    std::unordered_map<std::string, size_t> action_count_by_signature;

    if (!ordered_actions.empty()) {
        summary_.fastest_speed = ordered_actions.front().speed;
        summary_.slowest_speed = ordered_actions.front().speed;
    }

    const auto addIssue = [&](BattleInspectorIssueSeverity severity, std::string code,
                              std::optional<size_t> row_index, std::string message) {
        BattleInspectorIssue issue;
        issue.severity = severity;
        issue.code = std::move(code);
        issue.action_order = row_index;
        issue.message = std::move(message);
        if (row_index.has_value() && row_index.value() < issue_count_by_row.size()) {
            ++issue_count_by_row[row_index.value()];
        }
        issues_.push_back(std::move(issue));
    };

    all_rows_.reserve(ordered_actions.size());
    for (size_t index = 0; index < ordered_actions.size(); ++index) {
        const auto& action = ordered_actions[index];
        BattleInspectorActionRow row;
        row.action_order = index;
        row.subject_id = action.subject_id;
        row.target_id = action.target_id;
        row.command = action.command;
        row.speed = action.speed;
        row.priority = action.priority;

        summary_.fastest_speed = std::max(summary_.fastest_speed, row.speed);
        summary_.slowest_speed = std::min(summary_.slowest_speed, row.speed);

        if (!row.subject_id.empty()) {
            unique_subjects.insert(row.subject_id);
        }

        if (row.subject_id.empty()) {
            addIssue(BattleInspectorIssueSeverity::Error, "missing_subject", index,
                     "Queued action is missing subject id.");
        }
        if (row.target_id.empty()) {
            addIssue(BattleInspectorIssueSeverity::Warning, "missing_target", index,
                     "Queued action has no explicit target id.");
        }
        if (row.command.empty()) {
            addIssue(BattleInspectorIssueSeverity::Error, "missing_command", index,
                     "Queued action has no command binding.");
        }
        if (row.speed < 0) {
            addIssue(BattleInspectorIssueSeverity::Warning, "negative_speed", index,
                     "Queued action speed is negative and may break expected ordering semantics.");
        }
        if (row.priority < -10 || row.priority > 10) {
            addIssue(BattleInspectorIssueSeverity::Warning, "priority_out_of_range", index,
                     "Queued action priority is outside expected native range [-10, 10].");
        }

        const std::string signature = row.subject_id + "|" + row.target_id + "|" + row.command;
        const size_t signature_count = ++action_count_by_signature[signature];
        if (signature_count > 1) {
            addIssue(BattleInspectorIssueSeverity::Warning, "duplicate_action_signature", index,
                     "Action signature repeats existing queued entry and may indicate duplicate enqueue.");
        }

        row.summary = ActionSummary(row);
        all_rows_.push_back(std::move(row));
    }

    if (flow_controller.phase() == urpg::battle::BattleFlowPhase::Action && ordered_actions.empty()) {
        addIssue(BattleInspectorIssueSeverity::Warning, "action_phase_empty_queue", std::nullopt,
                 "Battle is in action phase but action queue is empty.");
    }
    if (!flow_controller.isActive() && !ordered_actions.empty()) {
        addIssue(BattleInspectorIssueSeverity::Warning, "queued_actions_in_terminal_phase", std::nullopt,
                 "Battle queue still has actions while flow is already terminal.");
    }
    if (flow_controller.escapeFailures() > 0 && !flow_controller.canEscape()) {
        addIssue(BattleInspectorIssueSeverity::Warning, "escape_failures_without_escape_lane", std::nullopt,
                 "Escape failures recorded while escape is currently unavailable.");
    }

    for (size_t index = 0; index < all_rows_.size(); ++index) {
        all_rows_[index].issue_count = issue_count_by_row[index];
    }

    summary_.unique_subjects = unique_subjects.size();
    summary_.issue_count = issues_.size();
    RebuildVisibleRows();
}

void BattleInspectorModel::SetShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void BattleInspectorModel::SetSubjectFilter(std::optional<std::string> subject_filter) {
    subject_filter_ = std::move(subject_filter);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

const BattleInspectorSummary& BattleInspectorModel::Summary() const {
    return summary_;
}

const std::vector<BattleInspectorActionRow>& BattleInspectorModel::VisibleRows() const {
    return visible_rows_;
}

const std::vector<BattleInspectorIssue>& BattleInspectorModel::Issues() const {
    return issues_;
}

bool BattleInspectorModel::SelectRow(size_t row_index) {
    if (row_index >= visible_rows_.size()) {
        selected_row_index_.reset();
        return false;
    }
    selected_row_index_ = row_index;
    return true;
}

std::optional<std::string> BattleInspectorModel::SelectedSubjectId() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }
    return visible_rows_[selected_row_index_.value()].subject_id;
}

void BattleInspectorModel::RebuildVisibleRows() {
    visible_rows_.clear();
    visible_rows_.reserve(all_rows_.size());

    for (const auto& row : all_rows_) {
        if (subject_filter_.has_value() && row.subject_id != subject_filter_.value()) {
            continue;
        }
        if (show_issues_only_ && row.issue_count == 0) {
            continue;
        }
        visible_rows_.push_back(row);
    }
}

} // namespace urpg::editor
