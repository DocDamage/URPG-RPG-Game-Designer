#include <catch2/catch_test_macros.hpp>

#include "engine/core/message/ai_sync_coordinator.h"
#include "engine/core/message/mock_chat_service.h"

#include <memory>
#include <string>
#include <vector>

namespace {

urpg::ai::ChatbotComponent makeChatbot() {
    return urpg::ai::ChatbotComponent(std::make_shared<urpg::ai::MockChatService>());
}

} // namespace

TEST_CASE("LocalInMemoryCloudService rejects live cloud providers", "[ai][cloud]") {
    urpg::social::LocalInMemoryCloudService cloud;

    const auto result = cloud.initialize(urpg::social::CloudProvider::GenericHTTP, "api-key");

    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(cloud.isOnline());
    REQUIRE(result.message.find("only supports CloudProvider::LocalSimulated") != std::string::npos);
}

TEST_CASE("AISyncCoordinator syncs and restores history through local in-memory storage", "[ai][cloud]") {
    auto cloud = std::make_shared<urpg::social::LocalInMemoryCloudService>();
    const auto init = cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "");

    REQUIRE(init.success);
    REQUIRE(init.message.find("process-local memory") != std::string::npos);

    urpg::ai::AISyncCoordinator coordinator(cloud);

    auto source_chatbot = makeChatbot();
    source_chatbot.restoreHistory({
        {"system", "You are a village sage."},
        {"user", "Where is the shrine?"},
        {"assistant", "Beyond the cedar bridge."}
    });

    REQUIRE(coordinator.syncHistoryToCloud("profile_alpha", source_chatbot));

    auto restored_chatbot = makeChatbot();
    REQUIRE(coordinator.restoreHistoryFromCloud("profile_alpha", restored_chatbot));

    const auto& restored_history = restored_chatbot.getHistory();
    REQUIRE(restored_history.size() == 3);
    CHECK(restored_history[0].role == "system");
    CHECK(restored_history[0].content == "You are a village sage.");
    CHECK(restored_history[1].role == "user");
    CHECK(restored_history[1].content == "Where is the shrine?");
    CHECK(restored_history[2].role == "assistant");
    CHECK(restored_history[2].content == "Beyond the cedar bridge.");
}

TEST_CASE("AISyncCoordinator stays offline when no local cloud service is initialized", "[ai][cloud]") {
    auto cloud = std::make_shared<urpg::social::LocalInMemoryCloudService>();
    urpg::ai::AISyncCoordinator coordinator(cloud);

    auto chatbot = makeChatbot();
    chatbot.restoreHistory({{"user", "Hello"}});

    REQUIRE_FALSE(coordinator.syncHistoryToCloud("profile_alpha", chatbot));
    REQUIRE_FALSE(coordinator.restoreHistoryFromCloud("profile_alpha", chatbot));
    REQUIRE_FALSE(coordinator.checkForRemoteKnowledgeUpdates("project_alpha"));
}
