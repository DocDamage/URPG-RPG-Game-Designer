#pragma once

#include <nlohmann/json.hpp>

#include <string>
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

EditorErrorCardValidation validateEditorErrorCard(const EditorErrorCard& card);
nlohmann::json editorErrorCardJson(const EditorErrorCard& card);
nlohmann::json redactedEditorErrorSupportExport(const EditorErrorCard& card);
std::string copyEditorErrorDetails(const EditorErrorCard& card);

} // namespace urpg::editor
