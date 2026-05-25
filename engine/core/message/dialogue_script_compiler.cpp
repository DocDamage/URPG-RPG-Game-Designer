#include "engine/core/message/dialogue_script_compiler.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>
#include <unordered_set>

namespace urpg::message {

namespace {

std::string trim(std::string_view value) {
    size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }

    size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return std::string(value.substr(begin, end - begin));
}

std::string toLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool startsWith(std::string_view value, std::string_view prefix) {
    return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

void addDiagnostic(DialogueScriptCompileResult& result, size_t line, std::string code, std::string message) {
    result.diagnostics.push_back({line, DialogueScriptDiagnosticSeverity::Error, std::move(code), std::move(message)});
}

void appendBodyLine(DialoguePage& page, const std::string& body_line) {
    if (!page.body.empty()) {
        page.body += '\n';
    }
    page.body += body_line;
}

bool parseBool(std::string_view text, bool& value) {
    const auto normalized = toLowerCopy(trim(text));
    if (normalized == "true" || normalized == "yes" || normalized == "1") {
        value = true;
        return true;
    }
    if (normalized == "false" || normalized == "no" || normalized == "0") {
        value = false;
        return true;
    }
    return false;
}

bool parseInt(std::string_view text, int32_t& value) {
    const auto trimmed = trim(text);
    if (trimmed.empty()) {
        return false;
    }
    size_t cursor = 0;
    bool negative = false;
    if (trimmed[cursor] == '-') {
        negative = true;
        ++cursor;
    }
    if (cursor >= trimmed.size()) {
        return false;
    }
    int32_t parsed = 0;
    while (cursor < trimmed.size()) {
        const auto ch = static_cast<unsigned char>(trimmed[cursor]);
        if (!std::isdigit(ch)) {
            return false;
        }
        parsed = parsed * 10 + static_cast<int32_t>(ch - '0');
        ++cursor;
    }
    value = negative ? -parsed : parsed;
    return true;
}

bool parseChoiceLine(const std::string& text, ChoiceOption& option) {
    std::string payload = trim(std::string_view(text).substr(1));
    const auto separator = payload.find('|');
    if (separator == std::string::npos) {
        return false;
    }

    std::string id_and_flags = trim(std::string_view(payload).substr(0, separator));
    option.label = trim(std::string_view(payload).substr(separator + 1));
    option.enabled = true;
    option.disabled_reason.clear();

    const std::string disabled_marker = "[disabled:";
    const auto disabled_pos = id_and_flags.find(disabled_marker);
    if (disabled_pos != std::string::npos) {
        const auto close_pos = id_and_flags.find(']', disabled_pos);
        if (close_pos == std::string::npos) {
            return false;
        }
        option.enabled = false;
        const auto reason_begin = disabled_pos + disabled_marker.size();
        option.disabled_reason = trim(std::string_view(id_and_flags).substr(reason_begin, close_pos - reason_begin));
        id_and_flags = trim(std::string_view(id_and_flags).substr(0, disabled_pos));
    }

    option.id = id_and_flags;
    return !option.id.empty() && !option.label.empty();
}

void applyDirective(DialogueScriptCompileResult& result, DialoguePage& page, size_t line_number,
                    const std::string& directive_line) {
    const auto separator = directive_line.find(' ');
    const auto key =
        toLowerCopy(separator == std::string::npos ? trim(std::string_view(directive_line).substr(1))
                                                   : trim(std::string_view(directive_line).substr(1, separator - 1)));
    const auto value =
        separator == std::string::npos ? std::string{} : trim(std::string_view(directive_line).substr(separator + 1));

    if (key == "mode") {
        const auto normalized = toLowerCopy(value);
        if (normalized == "speaker") {
            page.variant.mode = MessagePresentationMode::Speaker;
            page.variant.tone = MessageTone::Portrait;
            page.variant.route_token = "speaker:speaker:portrait";
        } else if (normalized == "narration") {
            page.variant.mode = MessagePresentationMode::Narration;
            page.variant.tone = MessageTone::Neutral;
            page.variant.speaker.clear();
            page.variant.face_actor_id = 0;
            page.variant.route_token = "narration:narration:neutral";
        } else if (normalized == "system") {
            page.variant.mode = MessagePresentationMode::System;
            page.variant.tone = MessageTone::System;
            page.variant.route_token = "system:system:system";
            if (page.variant.speaker.empty()) {
                page.variant.speaker = "System";
            }
        } else {
            addDiagnostic(result, line_number, "unknown_mode", "Unknown dialogue script mode.");
        }
        return;
    }

    if (key == "speaker") {
        page.variant.speaker = value;
        return;
    }

    if (key == "face") {
        int32_t face_actor_id = 0;
        if (!parseInt(value, face_actor_id)) {
            addDiagnostic(result, line_number, "invalid_face", "Face actor id must be an integer.");
            return;
        }
        page.variant.face_actor_id = face_actor_id;
        return;
    }

    if (key == "wait") {
        bool wait = true;
        if (!parseBool(value, wait)) {
            addDiagnostic(result, line_number, "invalid_wait", "Wait directive must be true or false.");
            return;
        }
        page.wait_for_advance = wait;
        return;
    }

    addDiagnostic(result, line_number, "unknown_directive", "Unknown dialogue script directive.");
}

} // namespace

bool DialogueScriptCompileResult::ok() const {
    return std::none_of(diagnostics.begin(), diagnostics.end(), [](const DialogueScriptDiagnostic& diagnostic) {
        return diagnostic.severity == DialogueScriptDiagnosticSeverity::Error;
    });
}

DialogueScriptCompileResult compileDialogueScript(const std::string& source) {
    DialogueScriptCompileResult result;
    std::unordered_set<std::string> page_ids;
    DialoguePage* current_page = nullptr;

    std::istringstream stream(source);
    std::string line;
    size_t line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        const std::string text = trim(line);
        if (text.empty() || startsWith(text, "#")) {
            continue;
        }

        if (startsWith(text, "::")) {
            const std::string page_id = trim(std::string_view(text).substr(2));
            if (page_id.empty()) {
                addDiagnostic(result, line_number, "missing_page_id", "Dialogue page header requires an id.");
                current_page = nullptr;
                continue;
            }
            if (!page_ids.insert(page_id).second) {
                addDiagnostic(result, line_number, "duplicate_page_id", "Dialogue page id is duplicated.");
            }

            DialoguePage page;
            page.id = page_id;
            page.variant = variantFromCompatRoute("speaker", "", 0);
            result.pages.push_back(std::move(page));
            current_page = &result.pages.back();
            continue;
        }

        if (!current_page) {
            if (startsWith(text, "-")) {
                addDiagnostic(result, line_number, "choice_without_page", "Choice line appears before a page header.");
            } else {
                addDiagnostic(result, line_number, "line_without_page",
                              "Dialogue content appears before a page header.");
            }
            continue;
        }

        if (startsWith(text, "@")) {
            applyDirective(result, *current_page, line_number, text);
            continue;
        }

        if (startsWith(text, "?")) {
            appendBodyLine(*current_page, trim(std::string_view(text).substr(1)));
            continue;
        }

        if (startsWith(text, "-")) {
            ChoiceOption option;
            if (!parseChoiceLine(text, option)) {
                addDiagnostic(result, line_number, "invalid_choice", "Choice line must use '- target | Label'.");
                continue;
            }
            current_page->choices.push_back(std::move(option));
            continue;
        }

        if (startsWith(text, "!")) {
            if (!current_page->command.empty()) {
                current_page->command += '\n';
            }
            current_page->command += trim(std::string_view(text).substr(1));
            continue;
        }

        if (startsWith(text, ">")) {
            current_page->variant = variantFromCompatRoute("narration", "", 0);
            appendBodyLine(*current_page, trim(std::string_view(text).substr(1)));
            continue;
        }

        const auto colon = text.find(':');
        if (colon != std::string::npos) {
            const auto speaker = trim(std::string_view(text).substr(0, colon));
            const auto body = trim(std::string_view(text).substr(colon + 1));
            if (speaker.empty()) {
                addDiagnostic(result, line_number, "missing_speaker", "Speaker line requires a speaker name.");
                continue;
            }
            current_page->variant = variantFromCompatRoute("speaker", speaker, current_page->variant.face_actor_id);
            appendBodyLine(*current_page, body);
            continue;
        }

        appendBodyLine(*current_page, text);
    }

    return result;
}

} // namespace urpg::message
