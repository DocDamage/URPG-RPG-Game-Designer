#pragma once

#include "editor/message/message_inspector_model.h"

namespace urpg::editor {

class MessageInspectorPanel {
public:
    struct RenderSnapshot {
        bool has_data = false;
        size_t total_pages = 0;
        size_t visible_row_count = 0;
        size_t issue_count = 0;
        std::string selected_page_id;
        std::vector<MessageInspectorRow> visible_rows;
        std::vector<MessageInspectorIssue> issues;
        std::optional<urpg::message::MessagePresentationMode> route_filter;
        bool show_issues_only = false;
        MessageInspectorSummary summary;
    };

    MessageInspectorPanel() = default;

    void bindRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                     const urpg::message::RichTextLayoutEngine& layout_engine);
    void clearRuntime();

    MessageInspectorModel& getModel();
    const MessageInspectorModel& getModel() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter);
    void setShowIssuesOnly(bool show_issues_only);

    void render();
    void refresh();
    void update();
    void clear();

    bool updatePageBody(size_t row, const std::string& body);
    bool addPage(const urpg::message::DialoguePage& page);
    bool removePage(size_t row);
    bool applyToRuntime(urpg::message::MessageFlowRunner& runner);

    bool hasRenderedFrame() const;
    const RenderSnapshot& lastRenderSnapshot() const;

private:
    const urpg::message::MessageFlowRunner* flow_runner_ = nullptr;
    const urpg::message::RichTextLayoutEngine* layout_engine_ = nullptr;
    MessageInspectorModel model_;
    bool visible_ = true;
    std::optional<urpg::message::MessagePresentationMode> route_filter_;
    bool show_issues_only_ = false;
    bool has_rendered_frame_ = false;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
