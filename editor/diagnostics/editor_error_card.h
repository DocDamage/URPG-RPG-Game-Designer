#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <optional>
#include <vector>

namespace urpg::editor {

struct EditorErrorAction {
    std::string label;
    std::string route;
    bool enabled = true;
};

struct EditorErrorCard {
    std::string code;
    std::string summary;
    std::string affected_object;
    std::string consequence;
    std::string suggested_fix;
    EditorErrorAction go_to;
    EditorErrorAction retry;
    nlohmann::json details = nlohmann::json::object();
};

struct EditorErrorCardValidation {
    bool valid = false;
    std::vector<std::string> issues;
};

struct EditorErrorCardRenderResult {
    bool go_to_requested = false;
    bool retry_requested = false;
    bool details_copied = false;
};

EditorErrorCardValidation validateEditorErrorCard(const EditorErrorCard& card);
nlohmann::json editorErrorCardJson(const EditorErrorCard& card);
std::optional<EditorErrorCard> editorErrorCardFromJson(const nlohmann::json& value);
nlohmann::json redactedEditorErrorSupportExport(const EditorErrorCard& card);
std::string copyEditorErrorDetails(const EditorErrorCard& card);
EditorErrorCardRenderResult renderEditorErrorCard(const EditorErrorCard& card);

} // namespace urpg::editor
