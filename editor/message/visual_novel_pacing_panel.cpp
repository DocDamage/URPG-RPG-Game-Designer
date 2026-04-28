#include "editor/message/visual_novel_pacing_panel.h"

#include <utility>

namespace urpg::editor {

void VisualNovelPacingPanel::loadDocument(urpg::message::VisualNovelPacingDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refreshPreview();
}

void VisualNovelPacingPanel::setAutoAdvance(bool enabled) {
    document_.controls.auto_advance = enabled;
    if (loaded_) {
        refreshPreview();
    }
}

void VisualNovelPacingPanel::setSkipReadText(bool enabled) {
    document_.controls.skip_read_text = enabled;
    if (loaded_) {
        refreshPreview();
    }
}

void VisualNovelPacingPanel::setTextSpeed(float characters_per_second) {
    document_.controls.text_speed_cps = characters_per_second;
    if (loaded_) {
        refreshPreview();
    }
}

void VisualNovelPacingPanel::appendBacklogEntry(urpg::message::VisualNovelBacklogEntry entry) {
    document_.backlog.push_back(std::move(entry));
    if (loaded_) {
        refreshPreview();
    }
}

void VisualNovelPacingPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load visual novel pacing controls before rendering this panel.";
        return;
    }
    refreshPreview();
}

void VisualNovelPacingPanel::refreshPreview() {
    preview_ = urpg::message::previewVisualNovelPacing(document_);
    snapshot_.disabled = false;
    snapshot_.document_id = document_.id;
    snapshot_.auto_advance_enabled = preview_.auto_advance_enabled;
    snapshot_.skip_available = preview_.skip_available;
    snapshot_.text_speed_cps = document_.controls.text_speed_cps;
    snapshot_.estimated_line_seconds = preview_.estimated_line_seconds;
    snapshot_.next_advance_seconds = preview_.next_advance_seconds;
    snapshot_.backlog_entry_count = preview_.backlog_entry_count;
    snapshot_.visible_backlog_count = preview_.visible_backlog_count;
    snapshot_.unread_entry_count = preview_.unread_entry_count;
    snapshot_.runtime_command_count = preview_.runtime_commands.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.next_action = preview_.next_action;
    if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Resolve visual-novel pacing diagnostics.";
    } else if (snapshot_.auto_advance_enabled) {
        snapshot_.ux_focus_lane = "auto_advance";
        snapshot_.primary_action = "Tune text speed and auto-advance delay.";
    } else if (snapshot_.skip_available) {
        snapshot_.ux_focus_lane = "skip";
        snapshot_.primary_action = "Preview skip-read behavior against the backlog.";
    } else {
        snapshot_.ux_focus_lane = "backlog";
        snapshot_.primary_action = "Review readable backlog history and advance input behavior.";
    }
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message = snapshot_.diagnostic_count == 0
        ? "Visual novel pacing preview is ready."
        : "Visual novel pacing preview has diagnostics.";
}

} // namespace urpg::editor
