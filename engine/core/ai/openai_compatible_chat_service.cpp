#include "engine/core/ai/openai_compatible_chat_service.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string_view>
#include <utility>

namespace {

std::string trim(std::string value) {
    const auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

std::pair<std::string, std::string> splitCommand(std::string content) {
    const std::vector<std::string> markers = {"\nCOMMAND:", "\nCommand:", "\ncommand:"};
    for (const auto& marker : markers) {
        const auto markerPos = content.find(marker);
        if (markerPos == std::string::npos) {
            continue;
        }
        auto response = trim(content.substr(0, markerPos));
        auto command = trim(content.substr(markerPos + marker.size()));
        const auto newline = command.find('\n');
        if (newline != std::string::npos) {
            command = trim(command.substr(0, newline));
        }
        return {response, command};
    }
    return {trim(std::move(content)), ""};
}

std::string firstTextFromContentArray(const nlohmann::json& content) {
    if (!content.is_array()) {
        return "";
    }
    std::string combined;
    for (const auto& part : content) {
        if (!part.is_object()) {
            continue;
        }
        if (part.contains("text") && part["text"].is_string()) {
            if (!combined.empty()) {
                combined += "\n";
            }
            combined += part["text"].get<std::string>();
        } else if (part.contains("content") && part["content"].is_string()) {
            if (!combined.empty()) {
                combined += "\n";
            }
            combined += part["content"].get<std::string>();
        }
    }
    return combined;
}

std::string streamedDeltaText(const nlohmann::json& response) {
    if (!response.is_object() || !response.contains("choices") || !response["choices"].is_array() ||
        response["choices"].empty()) {
        return "";
    }
    const auto& choice = response["choices"][0];
    if (!choice.is_object() || !choice.contains("delta") || !choice["delta"].is_object()) {
        return "";
    }
    const auto& delta = choice["delta"];
    if (delta.contains("content") && delta["content"].is_string()) {
        return delta["content"].get<std::string>();
    }
    return firstTextFromContentArray(delta.value("content", nlohmann::json::array()));
}

} // namespace

namespace urpg::ai {

nlohmann::json OpenAiCompatibleProviderProfile::toJson() const {
    return {{"id", id},
            {"label", label},
            {"endpoint", endpoint},
            {"default_model", default_model},
            {"local_provider", local_provider},
            {"api_key_required", api_key_required},
            {"streaming_supported", streaming_supported}};
}

nlohmann::json OpenAiCompatibleChatTransportResult::toJson() const {
    return {
        {"attempted", attempted},
        {"success", success},
        {"streaming_requested", streaming_requested},
        {"exit_code", exit_code},
        {"command", command},
        {"request_path", request_path},
        {"response_path", response_path},
        {"message", message},
        {"request_body", request_body},
        {"stream_diagnostics", stream_diagnostics},
    };
}

std::vector<OpenAiCompatibleProviderProfile> openAiCompatibleProviderProfiles() {
    return {
        {"chatgpt", "ChatGPT / OpenAI", "https://api.openai.com/v1/chat/completions", "gpt-5.5", false, true, true},
        {"openrouter", "OpenRouter", "https://openrouter.ai/api/v1/chat/completions", "openai/gpt-5.5", false, true,
         true},
        {"kimi", "Kimi / Moonshot", "https://api.moonshot.ai/v1/chat/completions", "moonshot-v1-8k", false, true, true},
        {"ollama", "Ollama", "http://127.0.0.1:11434/v1/chat/completions", "llama3.1", true, false, true},
        {"lm_studio", "LM Studio", "http://127.0.0.1:1234/v1/chat/completions", "local-model", true, false, true},
        {"vllm", "vLLM", "http://127.0.0.1:8000/v1/chat/completions", "local-model", true, false, true},
        {"localai", "LocalAI", "http://127.0.0.1:8080/v1/chat/completions", "local-model", true, false, true},
    };
}

OpenAiCompatibleProviderProfile openAiCompatibleProviderProfileById(const std::string& id) {
    const auto profiles = openAiCompatibleProviderProfiles();
    const auto it =
        std::find_if(profiles.begin(), profiles.end(), [&](const auto& profile) { return profile.id == id; });
    return it == profiles.end() ? profiles.front() : *it;
}

OpenAiCompatibleChatConfig applyOpenAiCompatibleProviderProfile(OpenAiCompatibleChatConfig config,
                                                                const OpenAiCompatibleProviderProfile& profile) {
    config.endpoint = profile.endpoint;
    config.model = profile.default_model;
    if (!profile.api_key_required) {
        config.api_key.clear();
    }
    return config;
}

nlohmann::json buildOpenAiCompatibleChatRequest(const std::vector<ChatMessage>& history,
                                                const OpenAiCompatibleChatConfig& config) {
    nlohmann::json messages = nlohmann::json::array();
    for (const auto& item : history) {
        messages.push_back({{"role", item.role.empty() ? "user" : item.role}, {"content", item.content}});
    }
    nlohmann::json request = {
        {"model", config.model.empty() ? "local-model" : config.model},
        {"temperature", config.temperature},
        {"messages", messages},
    };
    if (config.stream) {
        request["stream"] = true;
    }
    return request;
}

std::string buildOpenAiCompatibleChatCurlCommand(const OpenAiCompatibleChatConfig& config) {
    std::ostringstream command;
    command << "native_http_post";
    command << " endpoint=" << (config.endpoint.empty() ? "http://127.0.0.1:11434/v1/chat/completions"
                                                        : config.endpoint);
    command << " timeout_seconds=" << std::max(1, config.timeout_seconds);
    command << " request_path=" << (config.request_path.empty() ? "chat_request.json" : config.request_path);
    command << " response_path=" << (config.response_path.empty() ? "chat_response.json" : config.response_path);
    command << " stream=" << (config.stream ? "true" : "false");
    command << " auth=" << (config.api_key.empty() ? "none" : "[redacted]");
    return command.str();
}

nlohmann::json buildOpenAiCompatibleStreamAdapterPlan(const OpenAiCompatibleChatConfig& config) {
    const bool fixtureReplay = !config.execute && !config.response_path.empty();
    const bool liveHttpStream = config.execute && config.stream;
    return {
        {"component", "openai_compatible_stream_adapter"},
        {"stream_requested", config.stream},
        {"transport", fixtureReplay ? "fixture_response_replay"
                                     : (liveHttpStream ? "native_http_stream" : "native_http_buffered_request")},
        {"live_delivery", liveHttpStream},
        {"deterministic_replay", fixtureReplay},
        {"socket_adapter_ready", true},
        {"chunk_callback", "IChatService::StreamCallback"},
        {"completion_callback", "IChatService::ChatCallback"},
        {"command", buildOpenAiCompatibleChatCurlCommand(config)},
        {"request_path", config.request_path},
        {"response_path", config.response_path},
    };
}

std::pair<std::string, std::string> parseOpenAiCompatibleChatResponse(const nlohmann::json& response) {
    if (response.is_object() && response.contains("command") && response.contains("response") &&
        response["response"].is_string()) {
        return {response["response"].get<std::string>(), response.value("command", "")};
    }
    if (response.is_object() && response.contains("output_text") && response["output_text"].is_string()) {
        return splitCommand(response["output_text"].get<std::string>());
    }
    if (response.is_object() && response.contains("choices") && response["choices"].is_array() &&
        !response["choices"].empty()) {
        const auto& choice = response["choices"][0];
        if (choice.is_object() && choice.contains("message") && choice["message"].is_object()) {
            const auto& message = choice["message"];
            if (message.contains("content") && message["content"].is_string()) {
                return splitCommand(message["content"].get<std::string>());
            }
            const auto content = firstTextFromContentArray(message.value("content", nlohmann::json::array()));
            if (!content.empty()) {
                return splitCommand(content);
            }
        }
        if (choice.is_object() && choice.contains("text") && choice["text"].is_string()) {
            return splitCommand(choice["text"].get<std::string>());
        }
    }
    if (response.is_object() && response.contains("output") && response["output"].is_array()) {
        for (const auto& output : response["output"]) {
            if (output.is_object()) {
                const auto content = firstTextFromContentArray(output.value("content", nlohmann::json::array()));
                if (!content.empty()) {
                    return splitCommand(content);
                }
            }
        }
    }
    if (response.is_object() && response.contains("message") && response["message"].is_object()) {
        const auto& message = response["message"];
        if (message.contains("content") && message["content"].is_string()) {
            return splitCommand(message["content"].get<std::string>());
        }
    }
    return {"", ""};
}

std::pair<std::string, std::string> parseOpenAiCompatibleChatStreamResponse(std::string_view responseText) {
    const auto diagnostics = buildOpenAiCompatibleStreamDiagnostics(responseText);
    return {diagnostics.value("partial_text", ""), diagnostics.value("command", "")};
}

nlohmann::json buildOpenAiCompatibleStreamDiagnostics(std::string_view responseText) {
    std::istringstream input{std::string(responseText)};
    std::string line;
    std::string combined;
    nlohmann::json chunks = nlohmann::json::array();
    nlohmann::json errors = nlohmann::json::array();
    bool completed = false;
    bool cancelled = false;
    std::size_t lineIndex = 0;
    std::size_t chunkIndex = 0;
    while (std::getline(input, line)) {
        ++lineIndex;
        line = trim(std::move(line));
        if (line.empty()) {
            continue;
        }
        if (line.rfind("event:", 0) == 0 && trim(line.substr(std::string("event:").size())) == "cancelled") {
            cancelled = true;
            continue;
        }
        constexpr std::string_view prefix = "data:";
        if (line.rfind(prefix, 0) == 0) {
            line = trim(line.substr(prefix.size()));
        }
        if (line == "[DONE]") {
            completed = true;
            break;
        }
        try {
            const auto chunk = nlohmann::json::parse(line);
            if (chunk.is_object() && chunk.contains("error")) {
                const auto& error = chunk["error"];
                const auto message =
                    error.is_object() ? error.value("message", "provider_stream_error") : "provider_stream_error";
                errors.push_back({{"line", lineIndex}, {"message", message}, {"raw", chunk}});
                continue;
            }
            const auto delta = streamedDeltaText(chunk);
            if (!delta.empty()) {
                combined += delta;
                chunks.push_back({{"index", chunkIndex++},
                                  {"line", lineIndex},
                                  {"text", delta},
                                  {"partial_text", combined},
                                  {"sequence_ms", chunkIndex}});
                continue;
            }
            const auto parsed = parseOpenAiCompatibleChatResponse(chunk);
            if (!parsed.first.empty()) {
                combined += parsed.first;
                chunks.push_back({{"index", chunkIndex++},
                                  {"line", lineIndex},
                                  {"text", parsed.first},
                                  {"partial_text", combined},
                                  {"sequence_ms", chunkIndex}});
            }
        } catch (const nlohmann::json::exception&) {
            errors.push_back({{"line", lineIndex}, {"message", "stream_chunk_parse_failed"}, {"raw", line}});
        }
    }
    const auto parsed = splitCommand(combined);
    return {
        {"chunk_count", chunks.size()},
        {"chunks", chunks},
        {"partial_text", parsed.first},
        {"raw_partial_text", combined},
        {"command", parsed.second},
        {"completed", completed},
        {"cancelled", cancelled},
        {"provider_error", !errors.empty()},
        {"errors", errors},
        {"final_state",
         cancelled ? "cancelled" : (!errors.empty() ? "provider_error" : (completed ? "completed" : "open"))},
    };
}

OpenAiCompatibleChatTransportResult invokeOpenAiCompatibleChat(const std::vector<ChatMessage>& history,
                                                               const OpenAiCompatibleChatConfig& config,
                                                               urpg::net::IHttpClient* httpClient) {
    OpenAiCompatibleChatTransportResult result;
    result.streaming_requested = config.stream;
    result.request_path = config.request_path.empty() ? "chat_request.json" : config.request_path;
    result.response_path = config.response_path.empty() ? "chat_response.json" : config.response_path;
    result.request_body = buildOpenAiCompatibleChatRequest(history, config);
    result.command = buildOpenAiCompatibleChatCurlCommand(config);
    result.stream_diagnostics["adapter_plan"] = buildOpenAiCompatibleStreamAdapterPlan(config);
    if (!config.execute) {
        result.message = "dry_run";
        return result;
    }

    std::error_code requestDirError;
    const auto requestParent = std::filesystem::path(result.request_path).parent_path();
    if (!requestParent.empty()) {
        std::filesystem::create_directories(requestParent, requestDirError);
    }
    std::ofstream requestFile(result.request_path);
    if (!requestFile.is_open()) {
        result.message = "request_file_open_failed";
        return result;
    }
    requestFile << result.request_body.dump();
    requestFile.close();

    result.attempted = true;
    urpg::net::HttpRequest request;
    request.url = config.endpoint.empty() ? "http://127.0.0.1:11434/v1/chat/completions" : config.endpoint;
    request.headers = {{"Content-Type", "application/json"}};
    if (!config.api_key.empty()) {
        request.headers["Authorization"] = "Bearer " + config.api_key;
    }
    request.jsonBody = result.request_body;
    request.timeout = std::chrono::seconds(std::max(1, config.timeout_seconds));

    auto& client = httpClient == nullptr ? urpg::net::defaultHttpClient() : *httpClient;
    const auto response = client.postJson(request);
    result.exit_code = response.success() ? 0 : response.statusCode;
    if (response.success()) {
        std::error_code responseDirError;
        const auto responseParent = std::filesystem::path(result.response_path).parent_path();
        if (!responseParent.empty()) {
            std::filesystem::create_directories(responseParent, responseDirError);
        }
        std::ofstream responseFile(result.response_path, std::ios::binary | std::ios::trunc);
        if (!responseFile.is_open()) {
            result.message = "response_file_open_failed";
            return result;
        }
        responseFile << response.body;
        result.success = responseFile.good() && std::filesystem::exists(result.response_path);
        result.message = result.success ? "provider_response_written" : "provider_response_write_failed";
    } else {
        result.success = false;
        result.message = response.error.empty() ? "provider_http_failed" : response.error;
    }
    return result;
}

OpenAiCompatibleChatService::OpenAiCompatibleChatService(OpenAiCompatibleChatConfig config)
    : config_(std::move(config)) {}

std::shared_ptr<ChatRequestHandle>
OpenAiCompatibleChatService::requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) {
    auto handle = std::make_shared<ChatRequestHandle>();
    last_result_ = invokeOpenAiCompatibleChat(history, config_);
    if (!last_result_.success) {
        if (!handle->cancelled()) {
            callback(last_result_.message, "");
        }
        return handle;
    }

    std::ifstream responseFile(last_result_.response_path);
    if (!responseFile.is_open()) {
        if (!handle->cancelled()) {
            callback("provider_response_open_failed", "");
        }
        return handle;
    }
    nlohmann::json response;
    try {
        responseFile >> response;
    } catch (const nlohmann::json::exception&) {
        if (!handle->cancelled()) {
            callback("provider_response_parse_failed", "");
        }
        return handle;
    }
    const auto parsed = parseOpenAiCompatibleChatResponse(response);
    if (!handle->cancelled()) {
        callback(parsed.first.empty() ? "provider_response_empty" : parsed.first, parsed.second);
    }
    return handle;
}

std::shared_ptr<ChatRequestHandle>
OpenAiCompatibleChatService::requestStream(const std::vector<ChatMessage>& history, StreamCallback onChunk,
                                           ChatCallback onComplete) {
    auto handle = std::make_shared<ChatRequestHandle>();
    auto streamConfig = config_;
    streamConfig.stream = true;
    last_result_ = invokeOpenAiCompatibleChat(history, streamConfig);
    if (!last_result_.success) {
        if (!handle->cancelled()) {
            onComplete(last_result_.message, "");
        }
        return handle;
    }

    std::ifstream responseFile(last_result_.response_path);
    if (!responseFile.is_open()) {
        if (!handle->cancelled()) {
            onComplete("provider_response_open_failed", "");
        }
        return handle;
    }
    std::ostringstream buffer;
    buffer << responseFile.rdbuf();
    const auto responseText = buffer.str();

    try {
        const auto response = nlohmann::json::parse(responseText);
        const auto parsed = parseOpenAiCompatibleChatResponse(response);
        if (!parsed.first.empty() && !handle->cancelled()) {
            onChunk(parsed.first);
        }
        last_result_.stream_diagnostics = {
            {"chunk_count", parsed.first.empty() ? 0 : 1},
            {"chunks", parsed.first.empty() ? nlohmann::json::array()
                                            : nlohmann::json::array({{{"index", 0},
                                                                      {"line", 1},
                                                                      {"text", parsed.first},
                                                                      {"partial_text", parsed.first},
                                                                      {"sequence_ms", 1}}})},
            {"partial_text", parsed.first},
            {"raw_partial_text", parsed.first},
            {"command", parsed.second},
            {"completed", true},
            {"cancelled", false},
            {"provider_error", false},
            {"errors", nlohmann::json::array()},
            {"final_state", "completed"},
        };
        if (!handle->cancelled()) {
            onComplete(parsed.first.empty() ? "provider_response_empty" : parsed.first, parsed.second);
        }
        return handle;
    } catch (const nlohmann::json::exception&) {
    }

    last_result_.stream_diagnostics = buildOpenAiCompatibleStreamDiagnostics(responseText);
    for (const auto& chunk : last_result_.stream_diagnostics.value("chunks", nlohmann::json::array())) {
        if (chunk.is_object() && chunk.contains("text") && chunk["text"].is_string()) {
            if (!handle->cancelled()) {
                onChunk(chunk["text"].get<std::string>());
            }
        }
    }
    const auto parsed = parseOpenAiCompatibleChatStreamResponse(responseText);
    if (!handle->cancelled()) {
        onComplete(parsed.first.empty() ? "provider_response_empty" : parsed.first, parsed.second);
    }
    return handle;
}

} // namespace urpg::ai
