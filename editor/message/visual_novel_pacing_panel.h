#pragma once

#include "engine/core/message/visual_novel_pacing.h"

#include <string>

namespace urpg::editor {

struct VisualNovelPacingPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string document_id;
    bool auto_advance_enabled = false;
    bool skip_available = false;
    float text_speed_cps = 0.0f;
    float estimated_line_seconds = 0.0f;
    float next_advance_seconds = 0.0f;
    std::size_t backlog_entry_count = 0;
    std::size_t visible_backlog_count = 0;
    std::size_t unread_entry_count = 0;
    std::size_t runtime_command_count = 0;
    std::size_t diagnostic_count = 0;
    std::string ux_focus_lane = "pacing";
    std::string primary_action = "Load visual novel pacing controls.";
    std::string next_action;
    std::string saved_project_json;
    std::string status_message = "Load visual novel pacing controls before rendering this panel.";
};

class VisualNovelPacingPanel {
public:
    void loadDocument(urpg::message::VisualNovelPacingDocument document);
    void setAutoAdvance(bool enabled);
    void setSkipReadText(bool enabled);
    void setTextSpeed(float characters_per_second);
    void appendBacklogEntry(urpg::message::VisualNovelBacklogEntry entry);
    void render();

    [[nodiscard]] const VisualNovelPacingPanelSnapshot& snapshot() const { return snapshot_; }
    [[nodiscard]] const urpg::message::VisualNovelPacingPreview& preview() const { return preview_; }
    [[nodiscard]] bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::message::VisualNovelPacingDocument document_;
    urpg::message::VisualNovelPacingPreview preview_;
    VisualNovelPacingPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
