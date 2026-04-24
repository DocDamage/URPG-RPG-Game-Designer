#pragma once

#include "engine/core/message/message_core.h"
#include "window_compat.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::compat {

urpg::message::MessageAlignment parseMessageAlignment(const std::string& align);

urpg::message::RichTextLayoutEngine buildLayoutEngineForWindow(
    const Window_Base& window,
    int32_t maxWidth,
    urpg::message::MessageAlignment alignment);

int32_t measurePlainTextRenderer(const std::string& text, int32_t fontSize);

int32_t lineOffsetForFirstLine(const std::vector<urpg::message::RichTextToken>& tokens);

} // namespace urpg::compat
