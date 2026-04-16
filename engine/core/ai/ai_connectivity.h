#pragma once

#include "engine/core/message/chatbot_component.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace urpg::ai {

/**
 * @brief Implementation of IChatService that connects to the OpenAI Chat Completions API.
 */
class OpenAIChatService : public IChatService {
public:
    OpenAIChatService(const std::string& apiKey, const std::string& model = "gpt-4-turbo")
        : m_apiKey(apiKey), m_model(model) {}

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override {
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
 * @brief Implementation of IChatService for local inference via Llama.cpp.
 */
class LlamaLocalService : public IChatService {
public:
    LlamaLocalService(const std::string& modelPath) : m_modelPath(modelPath) {
        // Initialization of llama_context would happen here.
    }

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override {
        // Logic to run local inference using llama.cpp library calls.
        // callback("Local response from Llama.cpp", "");
    }

private:
    std::string m_modelPath;
};

} // namespace urpg::ai
