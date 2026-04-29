#include "engine/core/ai/openai_compatible_chat_service.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("OpenAI-compatible chat service builds request and curl command",
          "[ai][chat][provider]") {
    urpg::ai::OpenAiCompatibleChatConfig config;
    config.endpoint = "http://127.0.0.1:1234/v1/chat/completions";
    config.model = "local-test";
    config.api_key = "test-key";
    config.request_path = "tmp/chat-request.json";
    config.response_path = "tmp/chat-response.json";

    const std::vector<urpg::ai::ChatMessage> history = {
        {"system", "You are helpful."},
        {"user", "Plan a house."},
    };

    const auto request = urpg::ai::buildOpenAiCompatibleChatRequest(history, config);
    REQUIRE(request["model"] == "local-test");
    REQUIRE(request["messages"].size() == 2);
    REQUIRE(request["messages"][1]["role"] == "user");
    REQUIRE(request["messages"][1]["content"] == "Plan a house.");

    const auto command = urpg::ai::buildOpenAiCompatibleChatCurlCommand(config);
    REQUIRE(command.find("chat/completions") != std::string::npos);
    REQUIRE(command.find("Authorization: Bearer test-key") != std::string::npos);
    REQUIRE(command.find("tmp/chat-request.json") != std::string::npos);
    REQUIRE(command.find("tmp/chat-response.json") != std::string::npos);
}

TEST_CASE("OpenAI-compatible chat parser supports common response shapes",
          "[ai][chat][provider]") {
    const auto chat = urpg::ai::parseOpenAiCompatibleChatResponse({
        {"choices", nlohmann::json::array({
            {{"message", {{"role", "assistant"}, {"content", "Done.\nCOMMAND:AI_TASK:create dialogue"}}}},
        })},
    });
    REQUIRE(chat.first == "Done.");
    REQUIRE(chat.second == "AI_TASK:create dialogue");

    const auto responses = urpg::ai::parseOpenAiCompatibleChatResponse({
        {"output_text", "Use the export panel."},
    });
    REQUIRE(responses.first == "Use the export panel.");
    REQUIRE(responses.second.empty());

    const auto direct = urpg::ai::parseOpenAiCompatibleChatResponse({
        {"response", "Approved."},
        {"command", "AI_APPROVE_ALL"},
    });
    REQUIRE(direct.first == "Approved.");
    REQUIRE(direct.second == "AI_APPROVE_ALL");
}

TEST_CASE("OpenAI-compatible chat service dry run is deterministic",
          "[ai][chat][provider]") {
    urpg::ai::OpenAiCompatibleChatConfig config;
    config.execute = false;
    config.endpoint = "http://127.0.0.1:11434/v1/chat/completions";
    config.model = "llama-local";
    urpg::ai::OpenAiCompatibleChatService service(config);

    bool callbackCalled = false;
    service.requestResponse({{"user", "hello"}}, [&](const std::string& response, const std::string& command) {
        callbackCalled = true;
        REQUIRE(response == "dry_run");
        REQUIRE(command.empty());
    });

    REQUIRE(callbackCalled);
    REQUIRE_FALSE(service.lastTransportResult().attempted);
    REQUIRE_FALSE(service.lastTransportResult().success);
    REQUIRE(service.lastTransportResult().request_body["model"] == "llama-local");
}
