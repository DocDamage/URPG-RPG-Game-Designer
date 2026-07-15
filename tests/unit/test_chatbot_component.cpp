#include "engine/core/message/chatbot_component.h"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

class DelayedChatService : public urpg::ai::IChatService {
  public:
    std::shared_ptr<urpg::ai::ChatRequestHandle>
    requestResponse(const std::vector<urpg::ai::ChatMessage>& history, ChatCallback callback) override {
        auto handle = std::make_shared<urpg::ai::ChatRequestHandle>();
        pending.push_back({history, std::move(callback), handle});
        return handle;
    }

    void completeNext(const std::string& response, const std::string& command = "") {
        REQUIRE_FALSE(pending.empty());
        auto next = std::move(pending.front());
        pending.erase(pending.begin());
        next.callback(response, command);
    }

    struct PendingRequest {
        std::vector<urpg::ai::ChatMessage> history;
        ChatCallback callback;
        std::shared_ptr<urpg::ai::ChatRequestHandle> handle;
    };

    std::vector<PendingRequest> pending;
};

} // namespace

TEST_CASE("ChatbotComponent ignores delayed callbacks after destruction", "[chatbot][lifetime]") {
    auto service = std::make_shared<DelayedChatService>();
    int readyCount = 0;

    {
        urpg::ai::ChatbotComponent chatbot(service);
        chatbot.getResponse("hello", [&](urpg::message::DialoguePage) { ++readyCount; });
        REQUIRE(service->pending.size() == 1);
        REQUIRE(chatbot.getHistory().size() == 2);
    }

    service->completeNext("late answer", "AI_TASK:mutate");

    REQUIRE(readyCount == 0);
}

TEST_CASE("ChatbotComponent request cancellation leaves history unchanged", "[chatbot][lifetime]") {
    auto service = std::make_shared<DelayedChatService>();
    urpg::ai::ChatbotComponent chatbot(service);
    int readyCount = 0;

    auto handle = chatbot.getResponse("hold this", [&](urpg::message::DialoguePage) { ++readyCount; });
    REQUIRE(service->pending.size() == 1);
    REQUIRE(chatbot.getHistory().size() == 2);

    handle->cancel();
    service->completeNext("cancelled answer");

    REQUIRE(readyCount == 0);
    REQUIRE(chatbot.getHistory().size() == 2);
    REQUIRE(chatbot.getHistory().back().role == "user");
}
