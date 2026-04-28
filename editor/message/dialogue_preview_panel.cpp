#include "editor/message/dialogue_preview_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void DialoguePreviewPanel::loadDocument(urpg::message::DialoguePreviewDocument document,
                                        urpg::localization::LocaleCatalog locale_catalog) {
    document_ = std::move(document);
    locale_catalog_ = std::move(locale_catalog);
    selected_page_id_ = document_.pages.empty() ? "" : document_.pages.front().id;
    loaded_ = true;
    refreshPreview();
}

void DialoguePreviewPanel::selectPage(std::string page_id) {
    selected_page_id_ = std::move(page_id);
    if (loaded_) {
        refreshPreview();
    }
}

void DialoguePreviewPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a dialogue preview before rendering this panel.";
        return;
    }
    refreshPreview();
}

void DialoguePreviewPanel::refreshPreview() {
    preview_ = urpg::message::PreviewDialoguePage(document_, locale_catalog_, selected_page_id_);
    snapshot_.disabled = false;
    snapshot_.preview_id = document_.id;
    snapshot_.page_id = preview_.page_id;
    snapshot_.locale = preview_.locale;
    snapshot_.speaker = preview_.speaker;
    snapshot_.body = preview_.body;
    snapshot_.portrait_face_name = preview_.portrait ? preview_.portrait->face_name : "";
    snapshot_.portrait_face_index = preview_.portrait ? preview_.portrait->face_index : 0;
    snapshot_.choice_count = preview_.choices.size();
    snapshot_.enabled_choice_count = static_cast<size_t>(std::count_if(
        preview_.choices.begin(), preview_.choices.end(), [](const auto& choice) { return choice.enabled; }));
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.runtime_page_index = preview_.flow_snapshot.page_index;
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Dialogue preview is ready." : "Dialogue preview has diagnostics.";
}

} // namespace urpg::editor
