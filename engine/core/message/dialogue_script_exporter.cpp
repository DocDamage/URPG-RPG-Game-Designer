#include "engine/core/message/dialogue_script_exporter.h"

#include <sstream>
#include <string_view>

namespace urpg::message {

namespace {

std::vector<std::string> splitLines(std::string_view text) {
    std::vector<std::string> lines;
    size_t cursor = 0;
    while (cursor <= text.size()) {
        const auto next = text.find('\n', cursor);
        if (next == std::string_view::npos) {
            lines.emplace_back(text.substr(cursor));
            break;
        }
        lines.emplace_back(text.substr(cursor, next - cursor));
        cursor = next + 1;
    }
    if (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }
    return lines;
}

std::string modeName(MessagePresentationMode mode) {
    switch (mode) {
    case MessagePresentationMode::Speaker:
        return "speaker";
    case MessagePresentationMode::Narration:
        return "narration";
    case MessagePresentationMode::System:
        return "system";
    }
    return "speaker";
}

} // namespace

std::string exportDialogueScript(const std::vector<DialoguePage>& pages) {
    std::ostringstream out;

    for (size_t page_index = 0; page_index < pages.size(); ++page_index) {
        const auto& page = pages[page_index];
        if (page_index > 0) {
            out << '\n';
        }

        out << ":: " << page.id << '\n';
        if (page.variant.mode == MessagePresentationMode::System) {
            out << "@mode " << modeName(page.variant.mode) << '\n';
        }
        if (page.variant.face_actor_id > 0) {
            out << "@face " << page.variant.face_actor_id << '\n';
        }
        if (!page.wait_for_advance) {
            out << "@wait false\n";
        }

        const auto body_lines = splitLines(page.body);
        for (size_t line_index = 0; line_index < body_lines.size(); ++line_index) {
            const auto& body_line = body_lines[line_index];
            if (page.variant.mode == MessagePresentationMode::Narration) {
                out << "> " << body_line << '\n';
            } else if (line_index == 0) {
                const auto speaker = page.variant.speaker.empty() ? "Speaker" : page.variant.speaker;
                out << speaker << ": " << body_line << '\n';
            } else if (!page.choices.empty() && line_index == body_lines.size() - 1) {
                out << "? " << body_line << '\n';
            } else {
                out << body_line << '\n';
            }
        }

        for (const auto& choice : page.choices) {
            out << "- " << choice.id;
            if (!choice.enabled) {
                out << " [disabled: " << choice.disabled_reason << "]";
            }
            out << " | " << choice.label << '\n';
        }

        const auto command_lines = splitLines(page.command);
        for (const auto& command_line : command_lines) {
            out << "! " << command_line << '\n';
        }
    }

    return out.str();
}

} // namespace urpg::message
