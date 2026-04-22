#pragma once

#include "engine/core/message/chatbot_component.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace urpg::ai {

/**
 * @brief NOT LIVE: stub-shaped IChatService that only models a future OpenAI integration.
 * No HTTP transport, SSE handling, or callback delivery is implemented in-tree.
 */
class OpenAIChatService : public IChatService {
public:
    OpenAIChatService(const std::string& apiKey, const std::string& model = "gpt-4-turbo")
        : m_apiKey(apiKey), m_model(model) {}

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override {
        // NOT LIVE - simulated integration scaffold only. This function does not contact
        // a remote service and intentionally does not invoke the callback yet.
        static_cast<void>(callback);

        // In a production environment, this would use a C++ HTTP library like cpp-htlib or cURL.
        // For this implementation, we simulate the structured request/response and streaming support.
        
        nlohmann::json body;
        body["model"] = m_model;
        body["messages"] = nlohmann::json::array();
        
        for (const auto& msg : history) {
            body["messages"].push_back({
                {"role", msg.role},
                {"content", msg.content}
            });
        }

        // Simulating an asynchronous network request
        // This would involve:
        // 1. Constructing the HTTP POST request to https://api.openai.com/v1/chat/completions
        // 2. Adding headers: "Authorization: Bearer " + m_apiKey and "Content-Type: application/json"
        // 3. Handling the JSON response or the Server-Sent Events (SSE) stream for streaming support.
        
        // We trigger the callback with a simulated response for now to show the integration point.
        // callback("This is a simulated response from OpenAI.", "");
    }

private:
    std::string m_apiKey;
    std::string m_model;
};

/**
 * @brief NOT LIVE: stub-shaped IChatService for a future local llama.cpp backend.
 * No model execution or callback delivery is implemented in-tree.
 */
class LlamaLocalService : public IChatService {
public:
    LlamaLocalService(const std::string& modelPath) : m_modelPath(modelPath) {
        // Initialization of llama_context would happen here.
    }

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override {
        // NOT LIVE - local inference is not wired. Calls are currently a no-op.
        static_cast<void>(history);
        static_cast<void>(callback);

        // Logic to run local inference using llama.cpp library calls.
        // callback("Local response from Llama.cpp", "");
    }

private:
    std::string m_modelPath;
};

} // namespace urpg::ai
