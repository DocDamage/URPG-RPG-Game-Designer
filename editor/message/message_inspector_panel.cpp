#include "editor/message/message_inspector_panel.h"

namespace urpg::editor {

void MessageInspectorPanel::bindRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                                        const urpg::message::RichTextLayoutEngine& layout_engine) {
    flow_runner_ = &flow_runner;
    layout_engine_ = &layout_engine;
}

void MessageInspectorPanel::clearRuntime() {
    flow_runner_ = nullptr;
    layout_engine_ = nullptr;
    model_ = MessageInspectorModel{};
}

MessageInspectorModel& MessageInspectorPanel::getModel() {
    return model_;
}

const MessageInspectorModel& MessageInspectorPanel::getModel() const {
    return model_;
}

void MessageInspectorPanel::setVisible(bool visible) {
    visible_ = visible;
}

bool MessageInspectorPanel::isVisible() const {
    return visible_;
}

void MessageInspectorPanel::setRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter) {
    route_filter_ = route_filter;
    model_.SetRouteFilter(route_filter_);
}

void MessageInspectorPanel::setShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    model_.SetShowIssuesOnly(show_issues_only_);
}

void MessageInspectorPanel::render() {
    if (!visible_) {
        return;
    }
}

void MessageInspectorPanel::refresh() {
    if (!flow_runner_ || !layout_engine_) {
        return;
    }

    model_.LoadFromFlow(*flow_runner_, *layout_engine_);
    model_.SetRouteFilter(route_filter_);
    model_.SetShowIssuesOnly(show_issues_only_);
}

void MessageInspectorPanel::update() {
    refresh();
}

} // namespace urpg::editor
