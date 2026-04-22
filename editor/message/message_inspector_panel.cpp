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

    const auto& summary = model_.Summary();
    const auto& rows = model_.VisibleRows();
    const auto& issues = model_.Issues();

    last_render_snapshot_.summary = summary;
    last_render_snapshot_.has_data = summary.total_pages > 0 || !issues.empty();
    last_render_snapshot_.total_pages = summary.total_pages;
    last_render_snapshot_.visible_row_count = rows.size();
    last_render_snapshot_.issue_count = issues.size();
    last_render_snapshot_.visible_rows = rows;
    last_render_snapshot_.issues = issues;
    last_render_snapshot_.route_filter = route_filter_;
    last_render_snapshot_.show_issues_only = show_issues_only_;

    if (model_.SelectedPageId().has_value()) {
        last_render_snapshot_.selected_page_id = *model_.SelectedPageId();
    } else {
        last_render_snapshot_.selected_page_id.clear();
    }

    has_rendered_frame_ = true;
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

void MessageInspectorPanel::clear() {
    has_rendered_frame_ = false;
    last_render_snapshot_ = {};
}

bool MessageInspectorPanel::updatePageBody(size_t row, const std::string& body) {
    return model_.updatePageBody(row, body);
}

bool MessageInspectorPanel::addPage(const urpg::message::DialoguePage& page) {
    return model_.addPage(page);
}

bool MessageInspectorPanel::removePage(size_t row) {
    return model_.removePage(row);
}

bool MessageInspectorPanel::applyToRuntime(urpg::message::MessageFlowRunner& runner) {
    return model_.applyToRuntime(runner);
}

bool MessageInspectorPanel::hasRenderedFrame() const {
    return has_rendered_frame_;
}

const MessageInspectorPanel::RenderSnapshot& MessageInspectorPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
