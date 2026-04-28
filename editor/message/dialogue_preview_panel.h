#pragma once

#include "engine/core/message/dialogue_preview.h"

#include <string>

namespace urpg::editor {

struct DialoguePreviewPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string preview_id;
    std::string page_id;
    std::string locale;
    std::string speaker;
    std::string body;
    std::string portrait_face_name;
    int32_t portrait_face_index = 0;
    size_t choice_count = 0;
    size_t enabled_choice_count = 0;
    size_t diagnostic_count = 0;
    size_t runtime_page_index = 0;
    size_t selected_choice_index = 0;
    size_t runtime_command_count = 0;
    size_t variable_after_choice_count = 0;
    size_t body_character_count = 0;
    bool portrait_visible = false;
    bool has_branch_target = false;
    std::string confirmed_choice_id;
    std::string next_page_id;
    std::string ux_focus_lane = "dialogue";
    std::string choice_state_summary;
    std::string primary_action = "Load a dialogue preview.";
    std::string variables_after_choice_json;
    std::string saved_project_json;
    std::string status_message = "Load a dialogue preview before rendering this panel.";
};

class DialoguePreviewPanel {
public:
    void loadDocument(urpg::message::DialoguePreviewDocument document,
                      urpg::localization::LocaleCatalog locale_catalog);
    void selectPage(std::string page_id);
    void selectChoice(size_t choice_index);
    void confirmSelectedChoice(bool confirm);
    void render();

    const DialoguePreviewPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::message::DialoguePreviewResult& preview() const { return preview_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::message::DialoguePreviewDocument document_;
    urpg::localization::LocaleCatalog locale_catalog_;
    urpg::message::DialoguePreviewResult preview_;
    DialoguePreviewPanelSnapshot snapshot_{};
    std::string selected_page_id_;
    urpg::message::DialoguePreviewInteraction interaction_;
    bool loaded_ = false;
};

} // namespace urpg::editor
