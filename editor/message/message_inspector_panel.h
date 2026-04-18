#pragma once

#include "editor/message/message_inspector_model.h"

namespace urpg::editor {

class MessageInspectorPanel {
public:
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

private:
    const urpg::message::MessageFlowRunner* flow_runner_ = nullptr;
    const urpg::message::RichTextLayoutEngine* layout_engine_ = nullptr;
    MessageInspectorModel model_;
    bool visible_ = true;
    std::optional<urpg::message::MessagePresentationMode> route_filter_;
    bool show_issues_only_ = false;
};

} // namespace urpg::editor
