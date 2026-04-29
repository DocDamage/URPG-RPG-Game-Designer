#include "engine/core/ai/openai_compatible_chat_service.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace {

std::string quoteCommandArg(const std::string& value) {
    std::string escaped = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            escaped += "\\\"";
        } else {
            escaped += ch;
        }
    }
    escaped += "\"";
    return escaped;
}

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

} // namespace

namespace urpg::ai {

nlohmann::json OpenAiCompatibleChatTransportResult::toJson() const {
    return {
        {"attempted", attempted},
        {"success", success},
        {"exit_code", exit_code},
        {"command", command},
        {"request_path", request_path},
        {"response_path", response_path},
        {"message", message},
        {"request_body", request_body},
    };
}

nlohmann::json buildOpenAiCompatibleChatRequest(const std::vector<ChatMessage>& history,
                                                const OpenAiCompatibleChatConfig& config) {
    nlohmann::json messages = nlohmann::json::array();
    for (const auto& item : history) {
        messages.push_back({{"role", item.role.empty() ? "user" : item.role}, {"content", item.content}});
    }
    return {
        {"model", config.model.empty() ? "local-model" : config.model},
        {"temperature", config.temperature},
        {"messages", messages},
    };
}

std::string buildOpenAiCompatibleChatCurlCommand(const OpenAiCompatibleChatConfig& config) {
    std::ostringstream command;
    command << quoteCommandArg(config.curl_executable.empty() ? "curl" : config.curl_executable)
            << " --fail --silent --show-error"
            << " --max-time " << std::max(1, config.timeout_seconds)
            << " -X POST"
            << " -H " << quoteCommandArg("Content-Type: application/json");
    if (!config.api_key.empty()) {
        command << " -H " << quoteCommandArg("Authorization: Bearer " + config.api_key);
    }
    command << " --data-binary @" << quoteCommandArg(config.request_path.empty() ? "chat_request.json" : config.request_path)
            << " -o " << quoteCommandArg(config.response_path.empty() ? "chat_response.json" : config.response_path)
            << " " << quoteCommandArg(config.endpoint.empty() ? "http://127.0.0.1:11434/v1/chat/completions" : config.endpoint);
    return command.str();
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

OpenAiCompatibleChatTransportResult invokeOpenAiCompatibleChat(const std::vector<ChatMessage>& history,
                                                               const OpenAiCompatibleChatConfig& config) {
    OpenAiCompatibleChatTransportResult result;
    result.request_path = config.request_path.empty() ? "chat_request.json" : config.request_path;
    result.response_path = config.response_path.empty() ? "chat_response.json" : config.response_path;
    result.request_body = buildOpenAiCompatibleChatRequest(history, config);
    result.command = buildOpenAiCompatibleChatCurlCommand(config);
    if (!config.execute) {
        result.message = "dry_run";
        return result;
    }

    std::ofstream requestFile(result.request_path);
    if (!requestFile.is_open()) {
        result.message = "request_file_open_failed";
        return result;
    }
    requestFile << result.request_body.dump();
    requestFile.close();

    result.attempted = true;
    result.exit_code = std::system(result.command.c_str());
    result.success = result.exit_code == 0 && std::filesystem::exists(result.response_path);
    result.message = result.success ? "provider_response_written" : "provider_command_failed";
    return result;
}

OpenAiCompatibleChatService::OpenAiCompatibleChatService(OpenAiCompatibleChatConfig config)
    : config_(std::move(config)) {}

void OpenAiCompatibleChatService::requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) {
    last_result_ = invokeOpenAiCompatibleChat(history, config_);
    if (!last_result_.success) {
        callback(last_result_.message, "");
        return;
    }

    std::ifstream responseFile(last_result_.response_path);
    if (!responseFile.is_open()) {
        callback("provider_response_open_failed", "");
        return;
    }
    nlohmann::json response;
    try {
        responseFile >> response;
    } catch (const nlohmann::json::exception&) {
        callback("provider_response_parse_failed", "");
        return;
    }
    const auto parsed = parseOpenAiCompatibleChatResponse(response);
    callback(parsed.first.empty() ? "provider_response_empty" : parsed.first, parsed.second);
}

} // namespace urpg::ai
