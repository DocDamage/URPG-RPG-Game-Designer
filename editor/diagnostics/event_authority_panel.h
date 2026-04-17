#pragma once

#include "editor/diagnostics/event_authority_diagnostics.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg {

struct EventAuthorityPanelRow {
    std::string ts;
    std::string level;
    std::string event_id;
    std::string block_id;
    std::string mode;
    std::string operation;
    std::string error_code;
    std::string message;
    std::string summary;
};

class EventAuthorityPanelModel {
public:
    void LoadFromJsonl(std::string_view diagnostics_jsonl);
    void SetFilter(std::string_view event_id_filter);
    void SetLevelFilter(std::string_view level_filter);
    void SetModeFilter(std::string_view mode_filter);

    const std::vector<EventAuthorityPanelRow>& VisibleRows() const;

    bool SelectRow(size_t row_index);
    bool SelectNextRow();
    bool SelectPreviousRow();
    bool CanSelectNextRow() const;
    bool CanSelectPreviousRow() const;
    std::optional<size_t> SelectedRowIndex() const;
    std::optional<EventAuthorityPanelRow> SelectedRow() const;
    std::optional<EventNavigationTarget> SelectedNavigationTarget() const;

private:
    std::vector<EventAuthorityDiagnosticEntry> all_entries_;
    std::vector<EventAuthorityPanelRow> visible_rows_;
    std::optional<size_t> selected_row_index_;
    std::string event_id_filter_;
    std::string level_filter_;
    std::string mode_filter_;

    void RebuildVisibleRows();
};

class EventAuthorityPanel {
public:
    struct SelectedRowSnapshot {
        std::string ts;
        std::string level;
        std::string event_id;
        std::string block_id;
        std::string mode;
        std::string operation;
        std::string error_code;
        std::string message;
        std::string summary;
    };

    struct RenderSnapshot {
        size_t visible_rows = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool has_selection = false;
        bool has_data = false;
        std::vector<EventAuthorityPanelRow> visible_row_entries;
        bool can_select_next_row = false;
        bool can_select_previous_row = false;
        std::string event_id_filter;
        std::string level_filter;
        std::string mode_filter;
        std::optional<size_t> selected_row_index;
        std::optional<SelectedRowSnapshot> selected_row;
        std::optional<EventNavigationTarget> selected_navigation_target;
    };

    EventAuthorityPanel() = default;

    EventAuthorityPanelModel& getModel() { return model_; }
    const EventAuthorityPanelModel& getModel() const { return model_; }

    bool selectRow(size_t row_index);
    bool selectNextRow();
    bool selectPreviousRow();
    bool canSelectNextRow() const;
    bool canSelectPreviousRow() const;
    std::optional<size_t> selectedRowIndex() const;
    std::optional<EventAuthorityPanelRow> selectedRow() const;
    std::optional<EventNavigationTarget> selectedNavigationTarget() const;

    void ingestDiagnosticsJsonl(std::string_view diagnostics_jsonl);
    void clearDiagnostics();

    void setFilter(std::string_view event_id_filter);
    std::string getFilter() const { return event_id_filter_; }
    void setLevelFilter(std::string_view level_filter);
    std::string getLevelFilter() const { return level_filter_; }
    void setModeFilter(std::string_view mode_filter);
    std::string getModeFilter() const { return mode_filter_; }

    void setVisible(bool visible);
    bool isVisible() const;

    void render();
    void refresh();
    void update();

    bool hasRenderedFrame() const { return has_rendered_frame_; }
    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    EventAuthorityPanelModel model_;
    std::string diagnostics_jsonl_;
    std::string event_id_filter_;
    std::string level_filter_;
    std::string mode_filter_;
    bool visible_ = true;
    bool has_rendered_frame_ = false;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg
