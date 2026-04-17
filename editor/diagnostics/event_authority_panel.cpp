#include "editor/diagnostics/event_authority_panel.h"

namespace urpg {

namespace {

std::string BuildSummary(const EventAuthorityDiagnosticEntry& entry) {
    std::string summary = entry.error_code;
    if (!entry.mode.empty()) {
        summary += " [" + entry.mode + "]";
    }
    if (!entry.message.empty()) {
        summary += " " + entry.message;
    }
    return summary;
}

} // namespace

void EventAuthorityPanelModel::LoadFromJsonl(std::string_view diagnostics_jsonl) {
    all_entries_ = EventAuthorityDiagnosticsIndex::ParseJsonl(diagnostics_jsonl);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void EventAuthorityPanelModel::SetFilter(std::string_view event_id_filter) {
    event_id_filter_ = std::string(event_id_filter);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void EventAuthorityPanelModel::SetLevelFilter(std::string_view level_filter) {
    level_filter_ = std::string(level_filter);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void EventAuthorityPanelModel::SetModeFilter(std::string_view mode_filter) {
    mode_filter_ = std::string(mode_filter);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

const std::vector<EventAuthorityPanelRow>& EventAuthorityPanelModel::VisibleRows() const {
    return visible_rows_;
}

bool EventAuthorityPanelModel::SelectRow(size_t row_index) {
    if (row_index >= visible_rows_.size()) {
        selected_row_index_.reset();
        return false;
    }

    selected_row_index_ = row_index;
    return true;
}

bool EventAuthorityPanelModel::SelectNextRow() {
    if (visible_rows_.empty()) {
        selected_row_index_.reset();
        return false;
    }

    if (!selected_row_index_.has_value()) {
        selected_row_index_ = 0;
        return true;
    }

    if (selected_row_index_.value() + 1 >= visible_rows_.size()) {
        return false;
    }

    selected_row_index_ = selected_row_index_.value() + 1;
    return true;
}

bool EventAuthorityPanelModel::SelectPreviousRow() {
    if (visible_rows_.empty()) {
        selected_row_index_.reset();
        return false;
    }

    if (!selected_row_index_.has_value()) {
        selected_row_index_ = visible_rows_.size() - 1;
        return true;
    }

    if (selected_row_index_.value() == 0) {
        return false;
    }

    selected_row_index_ = selected_row_index_.value() - 1;
    return true;
}

bool EventAuthorityPanelModel::CanSelectNextRow() const {
    if (visible_rows_.empty()) {
        return false;
    }

    if (!selected_row_index_.has_value()) {
        return true;
    }

    return selected_row_index_.value() + 1 < visible_rows_.size();
}

bool EventAuthorityPanelModel::CanSelectPreviousRow() const {
    if (visible_rows_.empty()) {
        return false;
    }

    if (!selected_row_index_.has_value()) {
        return true;
    }

    return selected_row_index_.value() > 0;
}

std::optional<size_t> EventAuthorityPanelModel::SelectedRowIndex() const {
    return selected_row_index_;
}

std::optional<EventAuthorityPanelRow> EventAuthorityPanelModel::SelectedRow() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }

    return visible_rows_[selected_row_index_.value()];
}

std::optional<EventNavigationTarget> EventAuthorityPanelModel::SelectedNavigationTarget() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }

    const auto& row = visible_rows_[selected_row_index_.value()];
    return EventNavigationTarget{row.event_id, row.block_id};
}

void EventAuthorityPanelModel::RebuildVisibleRows() {
    visible_rows_.clear();

    for (const auto& entry : all_entries_) {
        if (!event_id_filter_.empty() && entry.event_id != event_id_filter_) {
            continue;
        }
        if (!level_filter_.empty() && entry.level != level_filter_) {
            continue;
        }
        if (!mode_filter_.empty() && entry.mode != mode_filter_) {
            continue;
        }

        EventAuthorityPanelRow row;
        row.ts = entry.ts;
        row.level = entry.level;
        row.event_id = entry.event_id;
        row.block_id = entry.block_id;
        row.mode = entry.mode;
        row.operation = entry.operation;
        row.error_code = entry.error_code;
        row.message = entry.message;
        row.summary = BuildSummary(entry);
        visible_rows_.push_back(std::move(row));
    }
}

void EventAuthorityPanel::ingestDiagnosticsJsonl(std::string_view diagnostics_jsonl) {
    if (diagnostics_jsonl.empty()) {
        return;
    }

    if (!diagnostics_jsonl_.empty() && diagnostics_jsonl_.back() != '\n') {
        diagnostics_jsonl_ += '\n';
    }
    diagnostics_jsonl_ += diagnostics_jsonl;
}

bool EventAuthorityPanel::selectRow(size_t row_index) {
    return model_.SelectRow(row_index);
}

bool EventAuthorityPanel::selectNextRow() {
    return model_.SelectNextRow();
}

bool EventAuthorityPanel::selectPreviousRow() {
    return model_.SelectPreviousRow();
}

bool EventAuthorityPanel::canSelectNextRow() const {
    return model_.CanSelectNextRow();
}

bool EventAuthorityPanel::canSelectPreviousRow() const {
    return model_.CanSelectPreviousRow();
}

std::optional<size_t> EventAuthorityPanel::selectedRowIndex() const {
    return model_.SelectedRowIndex();
}

std::optional<EventAuthorityPanelRow> EventAuthorityPanel::selectedRow() const {
    return model_.SelectedRow();
}

std::optional<EventNavigationTarget> EventAuthorityPanel::selectedNavigationTarget() const {
    return model_.SelectedNavigationTarget();
}

void EventAuthorityPanel::clearDiagnostics() {
    diagnostics_jsonl_.clear();
    model_.LoadFromJsonl({});
    if (!event_id_filter_.empty()) {
        model_.SetFilter(event_id_filter_);
    }
}

void EventAuthorityPanel::setFilter(std::string_view event_id_filter) {
    event_id_filter_ = std::string(event_id_filter);
    model_.SetFilter(event_id_filter_);
}

void EventAuthorityPanel::setLevelFilter(std::string_view level_filter) {
    level_filter_ = std::string(level_filter);
    model_.SetLevelFilter(level_filter_);
}

void EventAuthorityPanel::setModeFilter(std::string_view mode_filter) {
    mode_filter_ = std::string(mode_filter);
    model_.SetModeFilter(mode_filter_);
}

void EventAuthorityPanel::setVisible(bool visible) {
    visible_ = visible;
}

bool EventAuthorityPanel::isVisible() const {
    return visible_;
}

void EventAuthorityPanel::render() {
    if (!visible_) {
        return;
    }

    size_t warning_count = 0;
    size_t error_count = 0;
    const auto& rows = model_.VisibleRows();
    for (const auto& row : rows) {
        if (row.level == "error") {
            ++error_count;
        } else {
            ++warning_count;
        }
    }

    std::optional<SelectedRowSnapshot> selected_row_snapshot;
    if (const auto selected_row = model_.SelectedRow(); selected_row.has_value()) {
        selected_row_snapshot = SelectedRowSnapshot{
            selected_row->ts,
            selected_row->level,
            selected_row->event_id,
            selected_row->block_id,
            selected_row->mode,
            selected_row->operation,
            selected_row->error_code,
            selected_row->message,
            selected_row->summary,
        };
    }

    last_render_snapshot_ = {
        rows.size(),
        warning_count,
        error_count,
        model_.SelectedNavigationTarget().has_value(),
        !rows.empty(),
        rows,
        model_.CanSelectNextRow(),
        model_.CanSelectPreviousRow(),
        event_id_filter_,
        level_filter_,
        mode_filter_,
        model_.SelectedRowIndex(),
        selected_row_snapshot,
        model_.SelectedNavigationTarget()
    };
    has_rendered_frame_ = true;
}

void EventAuthorityPanel::refresh() {
    model_.LoadFromJsonl(diagnostics_jsonl_);
    if (!event_id_filter_.empty()) {
        model_.SetFilter(event_id_filter_);
    }
    if (!level_filter_.empty()) {
        model_.SetLevelFilter(level_filter_);
    }
    if (!mode_filter_.empty()) {
        model_.SetModeFilter(mode_filter_);
    }
}

void EventAuthorityPanel::update() {
    refresh();
}

} // namespace urpg
