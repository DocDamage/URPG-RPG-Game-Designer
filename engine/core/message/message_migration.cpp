#include "engine/core/message/message_migration.h"

#include "engine/core/message/message_core.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <set>

namespace urpg::message {

namespace {

using json = nlohmann::json;

std::string SeverityLabel(MessageMigrationSeverity severity) {
    switch (severity) {
    case MessageMigrationSeverity::Info:
        return "info";
    case MessageMigrationSeverity::Warning:
        return "warn";
    case MessageMigrationSeverity::Error:
        return "error";
    }
    return "info";
}

std::string NormalizeRoute(std::string route) {
    std::transform(route.begin(), route.end(), route.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    if (route == "speaker" || route == "narration" || route == "system") {
        return route;
    }
    return "unsupported";
}

std::string StringifyScalar(const json& value) {
    if (value.is_string()) {
        return value.get<std::string>();
    }
    if (value.is_number_integer()) {
        return std::to_string(value.get<int64_t>());
    }
    if (value.is_number_float()) {
        return std::to_string(value.get<double>());
    }
    if (value.is_boolean()) {
        return value.get<bool>() ? "true" : "false";
    }
    return {};
}

template <typename T>
T SafeValue(const json& object, const char* key, T fallback) {
    if (!object.is_object()) {
        return fallback;
    }
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return fallback;
    }
    try {
        return it->get<T>();
    } catch (const json::exception&) {
        return fallback;
    }
}

bool IsKnownEscapeCommand(char command) {
    static const std::set<char> known_commands = {'C', 'I', 'V', 'N', 'P', 'G', '{', '}', '\\'};
    return known_commands.count(command) > 0;
}

std::vector<std::string> CollectUnsupportedEscapes(const std::string& text) {
    std::vector<std::string> unsupported;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') {
            continue;
        }
        if (i + 1 >= text.size()) {
            continue;
        }
        const char raw = text[i + 1];
        const char command = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
        if (!IsKnownEscapeCommand(command)) {
            std::string token;
            token.push_back('\\');
            token.push_back(raw);
            unsupported.push_back(token);
        }
    }
    return unsupported;
}

} // namespace

MessageMigrationResult UpgradeCompatMessageDocument(const nlohmann::json& compat_document) {
    MessageMigrationResult result;
    result.dialogue_sequences = {
        {"_urpg_format_version", "1.0"},
        {"sequences", json::array()},
    };
    result.message_styles = {
        {"_urpg_format_version", "1.0"},
        {"styles", json::array(
            {
                {
                    {"id", "default"},
                    {"namebox", {{"visible", true}, {"margin_x", 12}, {"margin_y", 8}, {"max_width", 320}}},
                    {"portrait", {{"visible", true}, {"dock", "left"}, {"x", 16}, {"y", 12}, {"width", 144}, {"height", 144}}},
                },
            })},
    };

    const auto emit_diagnostic = [&](MessageMigrationSeverity severity, std::string code, std::string page_id,
                                     std::string message, std::string token = "") {
        MessageMigrationDiagnostic diagnostic;
        diagnostic.severity = severity;
        diagnostic.code = std::move(code);
        diagnostic.page_id = std::move(page_id);
        diagnostic.message = std::move(message);
        diagnostic.token = std::move(token);
        result.diagnostics.push_back(std::move(diagnostic));
    };

    std::vector<json> compat_pages;
    if (compat_document.is_object() && compat_document.contains("pages") && compat_document["pages"].is_array()) {
        for (const auto& page : compat_document["pages"]) {
            compat_pages.push_back(page);
        }
    } else if (compat_document.is_object()) {
        compat_pages.push_back(compat_document);
    } else {
        compat_pages.push_back(json::object());
        emit_diagnostic(MessageMigrationSeverity::Error, "invalid_compat_document", "compat_page_0",
                        "Compat message payload is not an object; creating safe fallback page.");
        result.used_safe_fallback = true;
    }

    json sequence;
    sequence["id"] = SafeValue<std::string>(compat_document, "id", "compat_import_sequence");
    sequence["source"] = "compat_import";
    sequence["pages"] = json::array();

    bool added_safe_style = false;
    for (size_t i = 0; i < compat_pages.size(); ++i) {
        const json& compat_page = compat_pages[i];
        const std::string page_id = SafeValue<std::string>(compat_page, "id", "compat_page_" + std::to_string(i));

        const std::string layout_mode = SafeValue<std::string>(compat_page, "layoutMode", "speaker");
        std::string raw_route = SafeValue<std::string>(compat_page, "route", layout_mode);
        std::string normalized_route = NormalizeRoute(raw_route);
        bool safe_text_only = false;

        if (normalized_route == "unsupported") {
            emit_diagnostic(MessageMigrationSeverity::Warning, "unsupported_route", page_id,
                            "Unsupported route '" + raw_route + "' mapped to narration safe fallback.");
            normalized_route = "narration";
            safe_text_only = true;
            result.used_safe_fallback = true;
        }

        const std::string default_speaker = SafeValue<std::string>(compat_page, "defaultSpeaker", "");
        const std::string speaker_default = SafeValue<std::string>(compat_page, "speaker", default_speaker);
        int32_t face_actor_id = SafeValue<int32_t>(compat_page, "faceActorId", 0);

        MessagePresentationVariant variant =
            variantFromCompatRoute(normalized_route, speaker_default.empty() && normalized_route == "system" ? "System" : speaker_default,
                                   face_actor_id);

        if (variant.mode == MessagePresentationMode::Speaker && variant.speaker.empty()) {
            emit_diagnostic(MessageMigrationSeverity::Warning, "missing_speaker", page_id,
                            "Speaker route is missing speaker name; using 'Unknown Speaker'.");
            variant.speaker = "Unknown Speaker";
        }
        if (variant.mode != MessagePresentationMode::Speaker && face_actor_id > 0) {
            emit_diagnostic(MessageMigrationSeverity::Warning, "portrait_conflict", page_id,
                            "Portrait actor id is ignored for narration/system routes.");
            variant.face_actor_id = 0;
            safe_text_only = true;
            result.used_safe_fallback = true;
        }

        std::string body;
        if (compat_page.contains("body")) {
            body = StringifyScalar(compat_page["body"]);
            if (body.empty() && !compat_page["body"].is_null()) {
                body = compat_page["body"].dump();
                emit_diagnostic(MessageMigrationSeverity::Warning, "body_shape_fallback", page_id,
                                "Message body was non-scalar; converted to readable JSON text.");
                safe_text_only = true;
                result.used_safe_fallback = true;
            }
        }

        if (body.empty()) {
            emit_diagnostic(MessageMigrationSeverity::Warning, "empty_body", page_id,
                            "Message body is empty after migration.");
        }

        for (const auto& unsupported_token : CollectUnsupportedEscapes(body)) {
            emit_diagnostic(MessageMigrationSeverity::Warning, "unsupported_escape", page_id,
                            "Unsupported escape token preserved in text output.", unsupported_token);
            safe_text_only = true;
            result.used_safe_fallback = true;
        }

        json choices = json::array();
        if (compat_page.contains("choices") && compat_page["choices"].is_array()) {
            size_t enabled_count = 0;
            for (size_t ci = 0; ci < compat_page["choices"].size(); ++ci) {
                const json& source_choice = compat_page["choices"][ci];
                json mapped_choice;
                if (source_choice.is_string()) {
                    mapped_choice["id"] = "choice_" + std::to_string(ci);
                    mapped_choice["label"] = source_choice.get<std::string>();
                    mapped_choice["enabled"] = true;
                    ++enabled_count;
                } else if (source_choice.is_object()) {
                    const std::string text_fallback = SafeValue<std::string>(source_choice, "text", "");
                    mapped_choice["id"] = SafeValue<std::string>(source_choice, "id", "choice_" + std::to_string(ci));
                    mapped_choice["label"] = SafeValue<std::string>(source_choice, "label", text_fallback);
                    mapped_choice["enabled"] = SafeValue<bool>(source_choice, "enabled", true);
                    if (!mapped_choice["enabled"].get<bool>()) {
                        mapped_choice["disabled_reason"] = SafeValue<std::string>(source_choice, "disabledReason", "");
                    } else {
                        ++enabled_count;
                    }
                } else {
                    emit_diagnostic(MessageMigrationSeverity::Warning, "unsupported_choice_shape", page_id,
                                    "Choice entry had unsupported shape and was dropped.");
                    continue;
                }

                if (mapped_choice["label"].get<std::string>().empty()) {
                    emit_diagnostic(MessageMigrationSeverity::Warning, "empty_choice_label", page_id,
                                    "Choice label is empty.");
                }
                choices.push_back(std::move(mapped_choice));
            }

            if (!choices.empty() && enabled_count == 0) {
                emit_diagnostic(MessageMigrationSeverity::Error, "choice_unreachable", page_id,
                                "All migrated choices are disabled; progression requires manual fix.");
                safe_text_only = true;
                result.used_safe_fallback = true;
            }
        }

        json page;
        page["id"] = page_id;
        page["style_id"] = safe_text_only ? "safe_text_only" : "default";
        page["presentation_mode"] = normalized_route;
        page["tone"] = (normalized_route == "speaker" ? "portrait" : (normalized_route == "narration" ? "neutral" : "system"));
        page["speaker"] = variant.speaker;
        page["face_actor_id"] = variant.face_actor_id;
        page["body"] = body;
        page["wait_for_advance"] = SafeValue<bool>(compat_page, "waitForAdvance", true);
        page["source_route"] = raw_route;
        if (!choices.empty()) {
            page["choices"] = std::move(choices);
        }

        sequence["pages"].push_back(std::move(page));

        if (safe_text_only && !added_safe_style) {
            result.message_styles["styles"].push_back(
                {
                    {"id", "safe_text_only"},
                    {"namebox", {{"visible", false}, {"margin_x", 0}, {"margin_y", 0}, {"max_width", 320}}},
                    {"portrait", {{"visible", false}, {"dock", "left"}, {"x", 0}, {"y", 0}, {"width", 0}, {"height", 0}}},
                });
            added_safe_style = true;
        }
    }

    result.dialogue_sequences["sequences"].push_back(std::move(sequence));
    return result;
}

std::string ExportMessageMigrationDiagnosticsJsonl(const MessageMigrationResult& result) {
    std::string jsonl;
    for (const auto& diagnostic : result.diagnostics) {
        json row = {
            {"level", SeverityLabel(diagnostic.severity)},
            {"subsystem", "message_migration"},
            {"code", diagnostic.code},
            {"page_id", diagnostic.page_id},
            {"message", diagnostic.message},
        };
        if (!diagnostic.token.empty()) {
            row["token"] = diagnostic.token;
        }
        if (!jsonl.empty()) {
            jsonl.push_back('\n');
        }
        jsonl += row.dump();
    }
    return jsonl;
}

} // namespace urpg::message
