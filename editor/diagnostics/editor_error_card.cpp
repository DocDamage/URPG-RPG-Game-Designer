#include "editor/diagnostics/editor_error_card.h"
#include "editor/ui/editor_widget_state.h"

#include <algorithm>
#include <cctype>
#include <utility>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool sensitiveKey(const std::string& key) {
    const auto value = lower(key);
    return value.find("token") != std::string::npos || value.find("password") != std::string::npos ||
           value.find("secret") != std::string::npos || value.find("authorization") != std::string::npos ||
           value.find("path") != std::string::npos || value.find("username") != std::string::npos;
}

bool absolutePathLike(const std::string& value) {
    return (value.size() > 2 && std::isalpha(static_cast<unsigned char>(value[0])) && value[1] == ':' &&
            (value[2] == '/' || value[2] == '\\')) || value.starts_with("/home/") || value.starts_with("/Users/");
}

nlohmann::json redact(const nlohmann::json& value) {
    if (value.is_object()) {
        auto result = nlohmann::json::object();
        for (const auto& [key, child] : value.items()) result[key] = sensitiveKey(key) ? nlohmann::json("[redacted]") : redact(child);
        return result;
    }
    if (value.is_array()) {
        auto result = nlohmann::json::array();
        for (const auto& child : value) result.push_back(redact(child));
        return result;
    }
    if (value.is_string() && absolutePathLike(value.get<std::string>())) return "[redacted-path]";
    return value;
}

} // namespace

EditorErrorCardValidation validateEditorErrorCard(const EditorErrorCard& card) {
    EditorErrorCardValidation result;
    const auto require = [&](const bool condition, const char* issue) {
        if (!condition) result.issues.emplace_back(issue);
    };
    require(!card.code.empty(), "error_code_missing");
    require(!card.summary.empty(), "plain_summary_missing");
    require(card.summary.find('\n') == std::string::npos && card.summary.find("std::") == std::string::npos,
            "plain_summary_contains_raw_diagnostic");
    require(!card.affected_object.empty(), "affected_object_missing");
    require(!card.consequence.empty(), "consequence_missing");
    require(!card.suggested_fix.empty(), "suggested_fix_missing");
    require(!card.go_to.label.empty() && !card.go_to.route.empty(), "go_to_action_missing");
    require(!card.retry.label.empty(), "retry_action_missing");
    require(card.details.is_object(), "copy_details_payload_invalid");
    result.valid = result.issues.empty();
    return result;
}

nlohmann::json editorErrorCardJson(const EditorErrorCard& card) {
    const auto validation = validateEditorErrorCard(card);
    return {
        {"schema", "urpg.editor_error.v1"}, {"valid", validation.valid}, {"issues", validation.issues},
        {"code", card.code}, {"summary", card.summary}, {"affected_object", card.affected_object},
        {"consequence", card.consequence}, {"suggested_fix", card.suggested_fix},
        {"go_to", {{"label", card.go_to.label}, {"route", card.go_to.route}, {"enabled", card.go_to.enabled}}},
        {"retry", {{"label", card.retry.label}, {"route", card.retry.route}, {"enabled", card.retry.enabled}}},
        {"copy_details", true}, {"support_export", "redacted"}, {"details", card.details},
    };
}

std::optional<EditorErrorCard> editorErrorCardFromJson(const nlohmann::json& value) {
    if (!value.is_object() || value.value("schema", "") != "urpg.editor_error.v1" ||
        !value.contains("go_to") || !value["go_to"].is_object() ||
        !value.contains("retry") || !value["retry"].is_object() ||
        !value.contains("details") || !value["details"].is_object()) {
        return std::nullopt;
    }
    EditorErrorCard card;
    card.code = value.value("code", "");
    card.summary = value.value("summary", "");
    card.affected_object = value.value("affected_object", "");
    card.consequence = value.value("consequence", "");
    card.suggested_fix = value.value("suggested_fix", "");
    card.go_to = {value["go_to"].value("label", ""), value["go_to"].value("route", ""),
                  value["go_to"].value("enabled", false)};
    card.retry = {value["retry"].value("label", ""), value["retry"].value("route", ""),
                  value["retry"].value("enabled", false)};
    card.details = value["details"];
    if (!validateEditorErrorCard(card).valid) return std::nullopt;
    return card;
}

nlohmann::json redactedEditorErrorSupportExport(const EditorErrorCard& card) {
    auto result = editorErrorCardJson(card);
    result["schema"] = "urpg.editor_error_support.v1";
    result["details"] = redact(card.details);
    result["redaction"] = {{"credentials", "removed"}, {"filesystem_paths", "removed"}};
    return result;
}

std::string copyEditorErrorDetails(const EditorErrorCard& card) {
    return redactedEditorErrorSupportExport(card).dump(2);
}

EditorErrorCardRenderResult renderEditorErrorCard(const EditorErrorCard& card) {
    EditorErrorCardRenderResult result;
#ifdef URPG_IMGUI_ENABLED
    if (!validateEditorErrorCard(card).valid) return result;
    ImGui::PushID(card.code.c_str());
    ImGui::BeginChild("ActionableError", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders |
                      ImGuiChildFlags_AutoResizeY);
    ImGui::TextWrapped("%s", card.summary.c_str());
    ImGui::TextDisabled("Affected: %s", card.affected_object.c_str());
    ImGui::TextWrapped("Consequence: %s", card.consequence.c_str());
    ImGui::TextWrapped("Suggested fix: %s", card.suggested_fix.c_str());
    const auto actionDescriptor = [](std::string id, std::string label, const bool enabled,
                                     const EditorWidgetIntent intent) {
        EditorWidgetDescriptor descriptor{
            std::move(id), std::move(label), EditorWidgetKind::Button, intent,
            enabled ? EditorWidgetVisualState::Normal : EditorWidgetVisualState::Disabled,
            enabled ? "" : "This recovery action is unavailable for the current error.", {}};
        descriptor.accessible_name = descriptor.label;
        descriptor.keyboard_action = "Enter or Space activates this recovery action.";
        descriptor.controller_action = "Confirm activates this recovery action.";
        descriptor.canvas_alternative = "Use the ordered diagnostic action list.";
        descriptor.focus_order = 0;
        return descriptor;
    };
    result.go_to_requested = renderEditorWidget(
        actionDescriptor("error.go_to", card.go_to.label, card.go_to.enabled, EditorWidgetIntent::Primary)).activated;
    ImGui::SameLine();
    result.retry_requested = renderEditorWidget(
        actionDescriptor("error.retry", card.retry.label, card.retry.enabled, EditorWidgetIntent::Secondary)).activated;
    ImGui::SameLine();
    if (renderEditorWidget(actionDescriptor("error.copy", "Copy Redacted Details", true,
                                            EditorWidgetIntent::Secondary)).activated) {
        const auto details = copyEditorErrorDetails(card);
        ImGui::SetClipboardText(details.c_str());
        result.details_copied = true;
    }
    ImGui::EndChild();
    ImGui::PopID();
#else
    (void)card;
#endif
    return result;
}

} // namespace urpg::editor
