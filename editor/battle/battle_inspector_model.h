#pragma once

#include "engine/core/battle/battle_core.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class BattleInspectorIssueSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct BattleInspectorIssue {
    BattleInspectorIssueSeverity severity = BattleInspectorIssueSeverity::Info;
    std::string code;
    std::optional<size_t> action_order;
    std::string message;
};

struct BattleInspectorActionRow {
    size_t action_order = 0;
    std::string subject_id;
    std::string target_id;
    std::string command;
    int32_t speed = 0;
    int32_t priority = 0;
    size_t issue_count = 0;
    std::string summary;
};

struct BattleInspectorSummary {
    std::string phase = "none";
    bool active = false;
    bool can_escape = false;
    int32_t turn_count = 0;
    int32_t escape_failures = 0;
    size_t total_actions = 0;
    size_t unique_subjects = 0;
    int32_t fastest_speed = 0;
    int32_t slowest_speed = 0;
    size_t issue_count = 0;
};

class BattleInspectorModel {
public:
    void LoadFromRuntime(const urpg::battle::BattleFlowController& flow_controller,
                         const urpg::battle::BattleActionQueue& action_queue);
    void SetShowIssuesOnly(bool show_issues_only);
    void SetSubjectFilter(std::optional<std::string> subject_filter);

    const BattleInspectorSummary& Summary() const;
    const std::vector<BattleInspectorActionRow>& VisibleRows() const;
    const std::vector<BattleInspectorIssue>& Issues() const;

    bool SelectRow(size_t row_index);
    std::optional<std::string> SelectedSubjectId() const;

private:
    void RebuildVisibleRows();

    std::vector<BattleInspectorActionRow> all_rows_;
    std::vector<BattleInspectorActionRow> visible_rows_;
    std::vector<BattleInspectorIssue> issues_;
    BattleInspectorSummary summary_;
    bool show_issues_only_ = false;
    std::optional<std::string> subject_filter_;
    std::optional<size_t> selected_row_index_;
};

} // namespace urpg::editor
