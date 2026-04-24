#include "window_text_layout.h"

#include "data_manager.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace urpg::compat {

namespace {

char32_t decodeUtf8Codepoint(const std::string& text, size_t& cursor) {
    if (cursor >= text.size()) {
        return U'\0';
    }

    const unsigned char lead = static_cast<unsigned char>(text[cursor]);
    if (lead < 0x80) {
        ++cursor;
        return static_cast<char32_t>(lead);
    }

    if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        if ((b1 & 0xC0) == 0x80) {
            cursor += 2;
            return static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
        }
    } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
            cursor += 3;
            return static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
        }
    } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        const unsigned char b3 = static_cast<unsigned char>(text[cursor + 3]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
            cursor += 4;
            return static_cast<char32_t>(
                ((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F)
            );
        }
    }

    ++cursor;
    return static_cast<char32_t>(lead);
}

int32_t rendererGlyphAdvance(char32_t cp, int32_t fontSize) {
    const double size = static_cast<double>(std::max(1, fontSize));
    if (cp == U'\t') {
        return static_cast<int32_t>(std::lround(size * 2.2));
    }
    if (cp == U' ') {
        return static_cast<int32_t>(std::lround(size * 0.35));
    }

    if (cp < 0x80) {
        const unsigned char ch = static_cast<unsigned char>(cp);
        if (std::isdigit(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.56));
        }
        if (std::ispunct(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.45));
        }
        if (std::isupper(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.62));
        }
        return static_cast<int32_t>(std::lround(size * 0.56));
    }

    // Treat CJK ranges as full-width glyphs.
    if ((cp >= 0x2E80 && cp <= 0x9FFF) || (cp >= 0xF900 && cp <= 0xFAFF)) {
        return static_cast<int32_t>(std::lround(size));
    }
    // Emoji and symbol-heavy planes tend to render wider.
    if (cp >= 0x1F000) {
        return static_cast<int32_t>(std::lround(size * 1.1));
    }
    // Fallback: proportional non-ASCII glyph.
    return static_cast<int32_t>(std::lround(size * 0.75));
}

} // namespace

urpg::message::MessageAlignment parseMessageAlignment(const std::string& align) {
    std::string normalized;
    normalized.reserve(align.size());
    for (const char ch : align) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    if (normalized == "center") {
        return urpg::message::MessageAlignment::Center;
    }
    if (normalized == "right") {
        return urpg::message::MessageAlignment::Right;
    }
    return urpg::message::MessageAlignment::Left;
}

urpg::message::RichTextLayoutEngine buildLayoutEngineForWindow(
    const Window_Base& window,
    int32_t maxWidth,
    urpg::message::MessageAlignment alignment) {
    urpg::message::RichTextLayoutEngine layout;
    layout.setBaseFontSize(window.fontSize());
    layout.setLineHeight(window.lineHeight());
    layout.setMaxWidth(maxWidth);
    layout.setAlignment(alignment);
    layout.setVariableResolver([](int32_t id) -> int32_t {
        return DataManager::instance().getVariable(id);
    });
    layout.setActorNameResolver([](int32_t id) -> std::string {
        if (const auto* actor = DataManager::instance().getActor(id)) {
            if (!actor->name.empty()) {
                return actor->name;
            }
        }
        return "Actor " + std::to_string(std::max(0, id));
    });
    layout.setPartyMemberResolver([](int32_t index) -> int32_t {
        return DataManager::instance().getPartyMember(index);
    });
    return layout;
}

int32_t measurePlainTextRenderer(const std::string& text, int32_t fontSize) {
    if (text.empty()) {
        return 0;
    }
    int32_t width = 0;
    size_t cursor = 0;
    while (cursor < text.size()) {
        const char32_t cp = decodeUtf8Codepoint(text, cursor);
        if (cp == U'\0' || cp == U'\r' || cp == U'\n') {
            continue;
        }
        width += rendererGlyphAdvance(cp, fontSize);
    }
    return width;
}

int32_t lineOffsetForFirstLine(const std::vector<urpg::message::RichTextToken>& tokens) {
    for (const auto& token : tokens) {
        if (token.type == urpg::message::RichTextTokenType::LineOffset) {
            return token.value;
        }
        if (token.type == urpg::message::RichTextTokenType::NewLine) {
            break;
        }
    }
    return 0;
}

} // namespace urpg::compat
