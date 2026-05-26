#include "editor/message/dialogue_preview_panel.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <utility>

namespace urpg::editor {
namespace {

urpg::wysiwyg::PreviewTraceRow makeTraceRow(const std::string& command, const std::string& source_id) {
    const auto separator = command.find(':');
    if (separator == std::string::npos) {
        return {command, source_id, command};
    }
    return {command.substr(0, separator), source_id, command.substr(separator + 1)};
}

} // namespace

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
    snapshot_.body_character_count = preview_.body.size();
    snapshot_.portrait_visible = preview_.portrait.has_value();
    snapshot_.confirmed_choice_id = preview_.confirmed_choice_id;
    snapshot_.next_page_id = preview_.next_page_id;
    snapshot_.has_branch_target = !snapshot_.next_page_id.empty();
    snapshot_.choice_state_summary = std::to_string(snapshot_.enabled_choice_count) + "/" +
                                     std::to_string(snapshot_.choice_count) + " choices enabled";
    if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Resolve dialogue preview diagnostics before recording this page.";
    } else if (snapshot_.choice_count > 0 && !snapshot_.has_branch_target) {
        snapshot_.ux_focus_lane = "choices";
        snapshot_.primary_action = "Select and confirm a choice to preview branch state.";
    } else if (!snapshot_.portrait_visible) {
        snapshot_.ux_focus_lane = "portrait";
        snapshot_.primary_action = "Assign a portrait layer for this speaker preview.";
    } else {
        snapshot_.ux_focus_lane = "localized_preview";
        snapshot_.primary_action = "Compare portrait, localized text, variables, and runtime commands.";
    }
    nlohmann::json variables_json = nlohmann::json::object();
    for (const auto& [key, value] : preview_.variables_after_choice) {
        variables_json[std::to_string(key)] = value;
    }
    snapshot_.variables_after_choice_json = variables_json.dump();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Dialogue preview is ready." : "Dialogue preview has diagnostics.";

    preview_session_ = {};
    preview_session_.route_id = "message/dialogue_preview";
    preview_session_.surface_id = "dialogue_preview";
    preview_session_.source_id = document_.id;
    preview_session_.mode =
        preview_.runtime_commands.empty() ? urpg::wysiwyg::PreviewMode::EditorOnly
                                          : urpg::wysiwyg::PreviewMode::RuntimeBacked;
    for (const auto& command : preview_.runtime_commands) {
        preview_session_.runtime_trace_rows.push_back(makeTraceRow(command, preview_.page_id));
    }
    preview_session_.summary_rows.push_back({"page", preview_.page_id, preview_.body});
    preview_session_.summary_rows.push_back({"speaker", preview_.page_id, preview_.speaker});
    preview_session_.summary_rows.push_back({"choices", preview_.page_id, snapshot_.choice_state_summary});
    if (!preview_.next_page_id.empty()) {
        preview_session_.summary_rows.push_back({"next_page", preview_.page_id, preview_.next_page_id});
    }
    for (const auto& diagnostic : preview_.diagnostics) {
        preview_session_.diagnostics.push_back(
            {diagnostic.code, diagnostic.message, diagnostic.page_id.empty() ? diagnostic.target : diagnostic.page_id, true});
    }
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::SavedData,
                                         !document_.id.empty(),
                                         document_.id.empty() ? "Dialogue preview document id is missing."
                                                              : "Dialogue preview document is saved."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::LivePreview,
                                         !preview_.page_id.empty(),
                                         preview_.page_id.empty() ? "No dialogue page was projected."
                                                                  : "Dialogue page was projected."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::RuntimeExecution,
                                         !preview_.runtime_commands.empty(),
                                         std::to_string(preview_.runtime_commands.size()) + " runtime command rows."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::Diagnostics,
                                         preview_.diagnostics.empty(),
                                         preview_.diagnostics.empty() ? "No dialogue preview blockers."
                                                                      : "Dialogue preview diagnostics are present."});
    preview_session_.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::Tests,
                                         true,
                                         "Covered by PreviewSession and dialogue preview tests."});
    preview_session_.recomputeConfidence();
}

} // namespace urpg::editor
