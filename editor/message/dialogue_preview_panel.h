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
    std::string saved_project_json;
    std::string status_message = "Load a dialogue preview before rendering this panel.";
};

class DialoguePreviewPanel {
public:
    void loadDocument(urpg::message::DialoguePreviewDocument document,
                      urpg::localization::LocaleCatalog locale_catalog);
    void selectPage(std::string page_id);
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
    bool loaded_ = false;
};

} // namespace urpg::editor
