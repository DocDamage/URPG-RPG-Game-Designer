#pragma once

#include "engine/core/message/chatbot_component.h"

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace urpg::ai {

struct OpenAiCompatibleChatConfig {
    bool execute = false;
    bool stream = false;
    std::string endpoint = "http://127.0.0.1:11434/v1/chat/completions";
    std::string model = "local-model";
    std::string api_key;
    std::string curl_executable = "curl";
    std::string request_path = "chat_request.json";
    std::string response_path = "chat_response.json";
    int timeout_seconds = 45;
    float temperature = 0.2f;
};

struct OpenAiCompatibleProviderProfile {
    std::string id;
    std::string label;
    std::string endpoint;
    std::string default_model;
    bool local_provider = false;
    bool api_key_required = true;
    bool streaming_supported = false;
    nlohmann::json toJson() const;
};

struct OpenAiCompatibleChatTransportResult {
    bool attempted = false;
    bool success = false;
    bool streaming_requested = false;
    int exit_code = -1;
    std::string command;
    std::string request_path;
    std::string response_path;
    std::string message;
    nlohmann::json request_body = nlohmann::json::object();
    nlohmann::json toJson() const;
};

std::vector<OpenAiCompatibleProviderProfile> openAiCompatibleProviderProfiles();
OpenAiCompatibleProviderProfile openAiCompatibleProviderProfileById(const std::string& id);
OpenAiCompatibleChatConfig applyOpenAiCompatibleProviderProfile(OpenAiCompatibleChatConfig config,
                                                                const OpenAiCompatibleProviderProfile& profile);
nlohmann::json buildOpenAiCompatibleChatRequest(const std::vector<ChatMessage>& history,
                                                const OpenAiCompatibleChatConfig& config);
std::string buildOpenAiCompatibleChatCurlCommand(const OpenAiCompatibleChatConfig& config);
std::pair<std::string, std::string> parseOpenAiCompatibleChatResponse(const nlohmann::json& response);
std::pair<std::string, std::string> parseOpenAiCompatibleChatStreamResponse(std::string_view responseText);
OpenAiCompatibleChatTransportResult invokeOpenAiCompatibleChat(const std::vector<ChatMessage>& history,
                                                               const OpenAiCompatibleChatConfig& config);

class OpenAiCompatibleChatService : public IChatService {
public:
    explicit OpenAiCompatibleChatService(OpenAiCompatibleChatConfig config);

    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override;
    void requestStream(const std::vector<ChatMessage>& history, StreamCallback onChunk,
                       ChatCallback onComplete) override;
    const OpenAiCompatibleChatTransportResult& lastTransportResult() const { return last_result_; }

private:
    OpenAiCompatibleChatConfig config_;
    OpenAiCompatibleChatTransportResult last_result_;
};

} // namespace urpg::ai
