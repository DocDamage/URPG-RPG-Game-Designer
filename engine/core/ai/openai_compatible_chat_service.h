#pragma once

#include "engine/core/message/chatbot_component.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::ai {

struct OpenAiCompatibleChatConfig {
    bool execute = false;
    std::string endpoint = "http://127.0.0.1:11434/v1/chat/completions";
    std::string model = "local-model";
    std::string api_key;
    std::string curl_executable = "curl";
    std::string request_path = "chat_request.json";
    std::string response_path = "chat_response.json";
    int timeout_seconds = 45;
    float temperature = 0.2f;
};

struct OpenAiCompatibleChatTransportResult {
    bool attempted = false;
    bool success = false;
    int exit_code = -1;
    std::string command;
    std::string request_path;
    std::string response_path;
    std::string message;
    nlohmann::json request_body = nlohmann::json::object();
    nlohmann::json toJson() const;
};

nlohmann::json buildOpenAiCompatibleChatRequest(const std::vector<ChatMessage>& history,
                                                const OpenAiCompatibleChatConfig& config);
std::string buildOpenAiCompatibleChatCurlCommand(const OpenAiCompatibleChatConfig& config);
std::pair<std::string, std::string> parseOpenAiCompatibleChatResponse(const nlohmann::json& response);
OpenAiCompatibleChatTransportResult invokeOpenAiCompatibleChat(const std::vector<ChatMessage>& history,
                                                               const OpenAiCompatibleChatConfig& config);

class OpenAiCompatibleChatService : public IChatService {
public:
    explicit OpenAiCompatibleChatService(OpenAiCompatibleChatConfig config);

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override;
    const OpenAiCompatibleChatTransportResult& lastTransportResult() const { return last_result_; }

private:
    OpenAiCompatibleChatConfig config_;
    OpenAiCompatibleChatTransportResult last_result_;
};

} // namespace urpg::ai
