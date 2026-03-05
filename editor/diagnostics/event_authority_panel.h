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

} // namespace urpg
