#pragma once

#include "engine/core/message/message_core.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class MessageInspectorIssueSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct MessageInspectorIssue {
    MessageInspectorIssueSeverity severity = MessageInspectorIssueSeverity::Info;
    std::string code;
    std::string page_id;
    std::string message;
};

struct MessageInspectorRow {
    size_t page_index = 0;
    std::string page_id;
    std::string route;
    std::string tone;
    std::string speaker;
    int32_t face_actor_id = 0;
    std::string body_preview;
    int32_t line_count = 1;
    int32_t preview_width = 0;
    int32_t preview_height = 0;
    bool has_choices = false;
    size_t choice_count = 0;
    size_t issue_count = 0;
};

struct MessageInspectorSummary {
    size_t total_pages = 0;
    size_t speaker_pages = 0;
    size_t narration_pages = 0;
    size_t system_pages = 0;
    size_t choice_pages = 0;
    size_t issue_count = 0;
    int32_t max_preview_width = 0;
    bool has_active_flow = false;
    size_t current_page_index = 0;
};

class MessageInspectorModel {
public:
    void LoadFromFlow(const urpg::message::MessageFlowRunner& flow_runner,
                      const urpg::message::RichTextLayoutEngine& layout_engine);
    void SetRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter);
    void SetShowIssuesOnly(bool show_issues_only);

    const MessageInspectorSummary& Summary() const;
    const std::vector<MessageInspectorRow>& VisibleRows() const;
    const std::vector<MessageInspectorIssue>& Issues() const;

    bool SelectRow(size_t row_index);
    std::optional<std::string> SelectedPageId() const;

private:
    void RebuildVisibleRows();
    void RestoreSelectionByPageId(const std::optional<std::string>& page_id);

    std::vector<MessageInspectorRow> all_rows_;
    std::vector<MessageInspectorRow> visible_rows_;
    std::vector<MessageInspectorIssue> issues_;
    MessageInspectorSummary summary_;
    std::optional<urpg::message::MessagePresentationMode> route_filter_;
    bool show_issues_only_ = false;
    std::optional<size_t> selected_row_index_;
};

} // namespace urpg::editor
