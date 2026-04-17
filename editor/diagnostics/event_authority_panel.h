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

    const std::vector<EventAuthorityPanelRow>& VisibleRows() const;

    bool SelectRow(size_t row_index);
    std::optional<EventNavigationTarget> SelectedNavigationTarget() const;

private:
    std::vector<EventAuthorityDiagnosticEntry> all_entries_;
    std::vector<EventAuthorityPanelRow> visible_rows_;
    std::optional<size_t> selected_row_index_;
    std::string event_id_filter_;

    void RebuildVisibleRows();
};

class EventAuthorityPanel {
public:
    struct RenderSnapshot {
        size_t visible_rows = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool has_selection = false;
        bool has_data = false;
    };

    EventAuthorityPanel() = default;

    EventAuthorityPanelModel& getModel() { return model_; }
    const EventAuthorityPanelModel& getModel() const { return model_; }

    void ingestDiagnosticsJsonl(std::string_view diagnostics_jsonl);
    void clearDiagnostics();

    void setFilter(std::string_view event_id_filter);
    std::string getFilter() const { return event_id_filter_; }

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
    bool visible_ = true;
    bool has_rendered_frame_ = false;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg
