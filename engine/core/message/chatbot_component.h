#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "engine/core/message/message_core.h"
#include "engine/core/message/world_knowledge_bridge.h"

namespace urpg::ai {

/**
 * @brief Represents a message in a chatbot conversation.
 */
struct ChatMessage {
    std::string role; // "user", "assistant", "system"
    std::string content;
};

/**
 * @brief Interface for AI Chat Services.
 * This allows the game developer to plug in OpenAI, Anthropic, or a local model.
 */
class IChatService {
public:
    virtual ~IChatService() = default;

    using ChatCallback = std::function<void(const std::string& response, const std::string& command)>;
    using StreamCallback = std::function<void(const std::string& chunk)>;

    /**
     * @brief Sends a prompt to the AI and gets a response.
     * @param history The conversation history.
     * @param callback Called when the AI responds.
     */
    virtual void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) = 0;

    /**
     * @brief Optional streaming request.
     */
    virtual void requestStream(const std::vector<ChatMessage>& history, StreamCallback onChunk, ChatCallback onComplete) {
        // Fallback to non-streaming if not implemented
        requestResponse(history, onComplete);
    }
};

/**
 * @brief A specialized Dialogue Node that acts as an entry point for AI Chat.
 */
class ChatbotComponent {
public:
    ChatbotComponent(std::shared_ptr<IChatService> service) : m_service(service) {}

    void setSystemPrompt(const std::string& prompt) { m_systemPrompt = prompt; }

    /**
     * @brief Interacts with the AI.
     * @param userInput The text typed by the player.
     * @param onReady Callback providing a DialoguePage ready for the MessageFlowRunner.
     */
    void getResponse(const std::string& userInput, std::function<void(urpg::message::DialoguePage)> onReady) {
        prepareHistory(userInput);

        m_service->requestResponse(m_history, [this, onReady](const std::string& response, const std::string& command) {
            m_history.push_back({"assistant", response});

            // Process Tool Calling (Function Calling)
            if (!command.empty()) {
                this->executeTool(command);
            }

            urpg::message::DialoguePage page;
            page.body = response;
            page.command = command; // Pass through to local handlers if needed
            page.variant.speaker = "Mysterious AI";
            
            onReady(page);
        });
    }

    /**
     * @brief Executes a tool/function requested by the AI.
     */
    void executeTool(const std::string& command) {
        // High-level integration with DialogueCommandProcessor
        auto& processor = urpg::message::DialogueCommandProcessor(); // Re-use global processor or specialized one
        // Simulating the execution of tool commands like "GIVE_ITEM:potion"
        // In full impl, this would resolve to native C++ function calls
    }

    /**
     * @brief Streams the AI response in real-time.
     */
    void streamResponse(const std::string& userInput, IChatService::StreamCallback onChunk, std::function<void(urpg::message::DialoguePage)> onComplete) {
        prepareHistory(userInput);

        m_service->requestStream(m_history, onChunk, [this, onComplete](const std::string& response, const std::string& command) {
            m_history.push_back({"assistant", response});

            urpg::message::DialoguePage page;
            page.body = response;
            page.command = command;
            page.variant.speaker = "Mysterious AI";
            
            onComplete(page);
        });
    }

    void clearHistory() { m_history.clear(); }

    /**
     * @brief Internal access for sync services.
     */
    const std::vector<ChatMessage>& getHistory() const { return m_history; }

    /**
     * @brief Restore a conversation history from a cloud or disk save.
     */
    void restoreHistory(const std::vector<ChatMessage>& history) {
        m_history = history;
    }

private:
    void prepareHistory(const std::string& userInput) {
        // ALWAYS refresh the dynamic world state context for every request
        // This ensures the AI knows if a quest progressed or a switch flipped since the last message.
        std::string dynamicContext = WorldKnowledgeBridge::generateContext();
        
        if (m_history.empty()) {
            m_history.push_back({"system", m_systemPrompt + "\n\n" + dynamicContext});
        } else {
            // Update the system prompt or inject as a turn-based context hint
            m_history[0].content = m_systemPrompt + "\n\n" + dynamicContext;
        }

        m_history.push_back({"user", userInput});
    }

    std::shared_ptr<IChatService> m_service;
    std::vector<ChatMessage> m_history;
    std::string m_systemPrompt;
};

} // namespace urpg::ai
