#pragma once

#include "engine/core/ui/ui_window.h"
#include <string>
#include <vector>

namespace urpg::ui {

/**
 * @brief A specialized UI component for the Chatbot interaction.
 * Comprises an input prompt and a scrolling history of AI responses.
 */
class ChatWindow : public UIWindow {
public:
    ChatWindow();
    virtual ~ChatWindow() = default;

    /**
     * @brief Updates the cursor animation for the input prompt.
     */
    void update(float dt) override;

    /**
     * @brief Draws both the response history and the current input buffer.
     */
    void draw(SpriteBatcher& batcher) override;

    /**
     * @brief Appends a new message (either from player or AI) to the history.
     */
    void addMessage(const std::string& speaker, const std::string& content);

    /**
     * @brief Updates the last message in history (useful for streaming chunks).
     */
    void updateLastMessage(const std::string& extraContent) {
        if (!m_history.empty()) m_history.back().content += extraContent;
    }

    /**
     * @brief Clears all conversation history from the UI.
     */
    void clearHistory() { m_history.clear(); }

    /**
     * @brief Sets the current player input text.
     */
    void setInputBuffer(const std::string& buffer) { m_inputBuffer = buffer; }

    /**
     * @brief Enables multi-line text wrapping for long AI responses.
     */
    void setWrappingEnabled(bool enabled) { m_wrappingEnabled = enabled; }

private:
    struct ChatMessage {
        std::string speaker;
        std::string content;
        uint32_t color = 0xFFFFFFFF;
    };
bool m_wrappingEnabled = true;
    
    std::vector<ChatMessage> m_history;
    std::string m_inputBuffer;
    float m_cursorTime = 0.0f;
    bool m_showCursor = true;
    int m_maxHistoryLines = 5;
};

} // namespace urpg::ui