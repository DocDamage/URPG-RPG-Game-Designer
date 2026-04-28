#include "editor/message/dialogue_preview_panel.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <utility>

namespace urpg::editor {

void DialoguePreviewPanel::loadDocument(urpg::message::DialoguePreviewDocument document,
                                        urpg::localization::LocaleCatalog locale_catalog) {
    document_ = std::move(document);
    locale_catalog_ = std::move(locale_catalog);
    selected_page_id_ = document_.pages.empty() ? "" : document_.pages.front().id;
    interaction_ = {};
    loaded_ = true;
    refreshPreview();
}

void DialoguePreviewPanel::selectPage(std::string page_id) {
    selected_page_id_ = std::move(page_id);
    interaction_ = {};
    if (loaded_) {
        refreshPreview();
    }
}

void DialoguePreviewPanel::selectChoice(size_t choice_index) {
    interaction_.selected_choice_index = choice_index;
    if (loaded_) {
        refreshPreview();
    }
}

void DialoguePreviewPanel::confirmSelectedChoice(bool confirm) {
    interaction_.confirm_selected_choice = confirm;
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
    preview_ = urpg::message::PreviewDialoguePage(document_, locale_catalog_, selected_page_id_, interaction_);
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
    snapshot_.selected_choice_index = preview_.selected_choice_index.value_or(preview_.flow_snapshot.selected_choice_index);
    snapshot_.runtime_command_count = preview_.runtime_commands.size();
    snapshot_.variable_after_choice_count = preview_.variables_after_choice.size();
    snapshot_.confirmed_choice_id = preview_.confirmed_choice_id;
    snapshot_.next_page_id = preview_.next_page_id;
    nlohmann::json variables_json = nlohmann::json::object();
    for (const auto& [key, value] : preview_.variables_after_choice) {
        variables_json[std::to_string(key)] = value;
    }
    snapshot_.variables_after_choice_json = variables_json.dump();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Dialogue preview is ready." : "Dialogue preview has diagnostics.";
}

} // namespace urpg::editor
