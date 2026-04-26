#include "chat_window.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/sprite_batcher.h"
#include <utility>

namespace urpg::ui {

ChatWindow::ChatWindow() {
    // Default Chat Window Layout in UI coordinate space
    m_position = {40.0f, 40.0f};
    m_size = {720.0f, 400.0f};
    m_zIndex = 0.85f;
    m_backOpacity = 0.9f;
}

void ChatWindow::update(float dt) {
    m_cursorTime += dt;
    if (m_cursorTime >= 0.5f) {
        m_showCursor = !m_showCursor;
        m_cursorTime = 0.0f;
    }
}

void ChatWindow::draw(SpriteBatcher& batcher) {
    if (!m_visible)
        return;

    // 1. Draw base window frame and background using the parent UIWindow logic.
    UIWindow::draw(batcher);

    // 2. Draw Conversation History (Scrolling upward from the bottom of history area)
    auto& layer = urpg::RenderLayer::getInstance();

    float startY = m_position.y + 20.0f;
    float lineHeight = 28.0f;

    // We only show the last N lines in the window to maintain space.
    size_t historyCount = m_history.size();
    size_t startIdx = (historyCount > (size_t)m_maxHistoryLines) ? historyCount - m_maxHistoryLines : 0;

    for (size_t i = startIdx; i < historyCount; ++i) {
        const auto& msg = m_history[i];

        urpg::TextCommand cmd;
        cmd.text = "[" + msg.speaker + "]: " + msg.content;
        cmd.x = m_position.x + 20.0f;
        cmd.y = startY + (i - startIdx) * lineHeight;
        cmd.zOrder = static_cast<int32_t>(m_zIndex + 0.1f);
        layer.submit(urpg::toFrameRenderCommand(cmd));
    }

    // 3. Draw Player Input Tooltip (Always at the bottom of the window)
    float inputLineY = m_position.y + m_size.y - 50.0f;

    // Draw an input line separator
    urpg::RectCommand line;
    line.x = m_position.x + 10.0f;
    line.y = inputLineY - 10.0f;
    line.w = m_size.x - 20.0f;
    line.h = 2.0f;
    line.r = 1.0f;
    line.g = 1.0f;
    line.b = 1.0f;
    line.a = 0.5f;
    line.zOrder = static_cast<int32_t>(m_zIndex + 0.05f);
    layer.submit(urpg::toFrameRenderCommand(line));

    urpg::TextCommand inputCmd;
    inputCmd.text = "Ask Guide: " + m_inputBuffer + (m_showCursor ? "_" : "");
    inputCmd.x = m_position.x + 20.0f;
    inputCmd.y = inputLineY;
    inputCmd.zOrder = static_cast<int32_t>(m_zIndex + 0.1f);
    layer.submit(urpg::toFrameRenderCommand(inputCmd));
}

void ChatWindow::addMessage(const std::string& speaker, const std::string& content) {
    m_history.push_back({speaker, content});

    // Auto-scroll logic: trim old history if it gets too large for memory (e.g., 50 lines MAX)
    if (m_history.size() > 50) {
        m_history.erase(m_history.begin());
    }
}

} // namespace urpg::ui
