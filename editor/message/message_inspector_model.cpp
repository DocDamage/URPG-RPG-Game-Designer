#include "editor/message/message_inspector_model.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace urpg::editor {

namespace {

std::string ModeLabel(urpg::message::MessagePresentationMode mode) {
    switch (mode) {
    case urpg::message::MessagePresentationMode::Speaker:
        return "speaker";
    case urpg::message::MessagePresentationMode::Narration:
        return "narration";
    case urpg::message::MessagePresentationMode::System:
        return "system";
    }
    return "speaker";
}

std::string ToneLabel(urpg::message::MessageTone tone) {
    switch (tone) {
    case urpg::message::MessageTone::Portrait:
        return "portrait";
    case urpg::message::MessageTone::Neutral:
        return "neutral";
    case urpg::message::MessageTone::System:
        return "system";
    }
    return "portrait";
}

std::string PreviewBody(std::string_view body, size_t max_chars = 120) {
    if (body.size() <= max_chars) {
        return std::string(body);
    }
    std::string preview(body.substr(0, max_chars));
    preview += "...";
    return preview;
}

} // namespace

void MessageInspectorModel::LoadFromFlow(const urpg::message::MessageFlowRunner& flow_runner,
                                         const urpg::message::RichTextLayoutEngine& layout_engine) {
    all_rows_.clear();
    visible_rows_.clear();
    issues_.clear();
    selected_row_index_.reset();
    summary_ = {};
    summary_.has_active_flow = flow_runner.isActive();
    summary_.current_page_index = flow_runner.currentPageIndex();

    std::unordered_map<std::string, size_t> issue_count_by_page_id;

    const auto addIssue = [&](MessageInspectorIssueSeverity severity, std::string code,
                              std::string page_id, std::string message) {
        MessageInspectorIssue issue;
        issue.severity = severity;
        issue.code = std::move(code);
        issue.page_id = std::move(page_id);
        issue.message = std::move(message);
        ++issue_count_by_page_id[issue.page_id];
        issues_.push_back(std::move(issue));
    };

    const auto& pages = flow_runner.pages();
    summary_.total_pages = pages.size();
    all_rows_.reserve(pages.size());

    for (size_t index = 0; index < pages.size(); ++index) {
        const auto& page = pages[index];
        const std::string fallback_page_id = "page_" + std::to_string(index);
        const std::string row_page_id = page.id.empty() ? fallback_page_id : page.id;
        const auto layout = layout_engine.layout(page.body);

        MessageInspectorRow row;
        row.page_index = index;
        row.page_id = row_page_id;
        row.route = ModeLabel(page.variant.mode);
        row.tone = ToneLabel(page.variant.tone);
        row.speaker = page.variant.speaker;
        row.face_actor_id = page.variant.face_actor_id;
        row.body_preview = PreviewBody(page.body);
        row.line_count = layout.metrics.line_count;
        row.preview_width = layout.metrics.width;
        row.preview_height = layout.metrics.height;
        row.has_choices = !page.choices.empty();
        row.choice_count = page.choices.size();

        switch (page.variant.mode) {
        case urpg::message::MessagePresentationMode::Speaker:
            ++summary_.speaker_pages;
            break;
        case urpg::message::MessagePresentationMode::Narration:
            ++summary_.narration_pages;
            break;
        case urpg::message::MessagePresentationMode::System:
            ++summary_.system_pages;
            break;
        }

        if (row.has_choices) {
            ++summary_.choice_pages;
        }
        summary_.max_preview_width = std::max(summary_.max_preview_width, row.preview_width);

        if (page.id.empty()) {
            addIssue(MessageInspectorIssueSeverity::Error, "missing_page_id", row_page_id,
                     "Dialogue page id is required for migration-safe references.");
        }
        if (page.body.empty()) {
            addIssue(MessageInspectorIssueSeverity::Warning, "empty_body", row_page_id,
                     "Dialogue page body is empty.");
        }
        if (page.variant.mode == urpg::message::MessagePresentationMode::Speaker &&
            page.variant.speaker.empty()) {
            addIssue(MessageInspectorIssueSeverity::Warning, "missing_speaker", row_page_id,
                     "Speaker route page is missing speaker binding.");
        }
        if (!page.choices.empty() &&
            std::none_of(page.choices.begin(), page.choices.end(),
                         [](const urpg::message::ChoiceOption& option) { return option.enabled; })) {
            addIssue(MessageInspectorIssueSeverity::Error, "choice_unreachable", row_page_id,
                     "All choices are disabled; continuation target is unreachable.");
        }
        if (layout.metrics.width > 640) {
            std::ostringstream oss;
            oss << "Preview width " << layout.metrics.width
                << " exceeds 640 and may overflow default message chrome.";
            addIssue(MessageInspectorIssueSeverity::Warning, "overflow_risk", row_page_id, oss.str());
        }

        all_rows_.push_back(std::move(row));
    }

    for (auto& row : all_rows_) {
        const auto it = issue_count_by_page_id.find(row.page_id);
        row.issue_count = it == issue_count_by_page_id.end() ? 0 : it->second;
    }

    summary_.issue_count = issues_.size();
    RebuildVisibleRows();
}

void MessageInspectorModel::SetRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter) {
    route_filter_ = route_filter;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void MessageInspectorModel::SetShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

const MessageInspectorSummary& MessageInspectorModel::Summary() const {
    return summary_;
}

const std::vector<MessageInspectorRow>& MessageInspectorModel::VisibleRows() const {
    return visible_rows_;
}

const std::vector<MessageInspectorIssue>& MessageInspectorModel::Issues() const {
    return issues_;
}

bool MessageInspectorModel::SelectRow(size_t row_index) {
    if (row_index >= visible_rows_.size()) {
        selected_row_index_.reset();
        return false;
    }
    selected_row_index_ = row_index;
    return true;
}

std::optional<std::string> MessageInspectorModel::SelectedPageId() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }
    return visible_rows_[selected_row_index_.value()].page_id;
}

void MessageInspectorModel::RebuildVisibleRows() {
    visible_rows_.clear();
    visible_rows_.reserve(all_rows_.size());

    for (const auto& row : all_rows_) {
        if (route_filter_.has_value()) {
            const std::string expected_route = ModeLabel(*route_filter_);
            if (row.route != expected_route) {
                continue;
            }
        }
        if (show_issues_only_ && row.issue_count == 0) {
            continue;
        }
        visible_rows_.push_back(row);
    }
}

} // namespace urpg::editor
