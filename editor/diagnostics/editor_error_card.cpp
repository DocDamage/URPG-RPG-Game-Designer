#include "editor/diagnostics/editor_error_card.h"

#include <algorithm>
#include <cctype>

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

} // namespace urpg::editor
