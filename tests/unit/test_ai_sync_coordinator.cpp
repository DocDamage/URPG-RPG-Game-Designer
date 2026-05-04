#include <catch2/catch_test_macros.hpp>

#include "engine/core/message/ai_sync_coordinator.h"
#include "engine/core/message/mock_chat_service.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

urpg::ai::ChatbotComponent makeChatbot() {
    return urpg::ai::ChatbotComponent(std::make_shared<urpg::ai::MockChatService>());
}

class FakeCloudService final : public urpg::social::ICloudService {
  public:
    urpg::social::CloudResult initialize(urpg::social::CloudProvider provider, const std::string& apiKey) override {
        static_cast<void>(provider);
        static_cast<void>(apiKey);
        online = true;
        return {true, "fake initialized", 1};
    }

    urpg::social::CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) override {
        if (!online) {
            return {false, "fake offline", 2};
        }
        if (failWrites) {
            return {false, "fake write rejected", 3};
        }
        storage[key] = data;
        return {true, "fake stored", 4};
    }

    std::vector<uint8_t> fetchFromCloud(const std::string& key) override {
        if (!online) {
            return {};
        }
        const auto it = storage.find(key);
        if (it == storage.end()) {
            return {};
        }
        return it->second;
    }

    std::vector<std::string> listRemoteKeys() override {
        std::vector<std::string> keys;
        for (const auto& [key, value] : storage) {
            static_cast<void>(value);
            keys.push_back(key);
        }
        return keys;
    }

    bool isOnline() const override { return online; }
    urpg::social::CloudResult getStatus() const override {
        return {online, online ? "fake online" : "fake offline", 5};
    }

    void putText(const std::string& key, const std::string& text) {
        storage[key] = std::vector<uint8_t>(text.begin(), text.end());
    }

    bool online = false;
    bool failWrites = false;
    std::map<std::string, std::vector<uint8_t>> storage;
};

} // namespace

TEST_CASE("LocalInMemoryCloudService initializes the local-only provider path", "[ai][cloud]") {
    urpg::social::LocalInMemoryCloudService cloud;

    const auto result = cloud.initialize(urpg::social::CloudProvider::LocalSimulated, "api-key");

    REQUIRE(result.success);
    REQUIRE(cloud.isOnline());
    REQUIRE(result.message.find("process-local memory") != std::string::npos);
}

TEST_CASE("LocalInMemoryCloudService is hidden from release cloud-sync surfaces", "[ai][cloud][release]") {
    urpg::social::LocalInMemoryCloudService cloud;

    const auto visibility = cloud.releaseVisibility();

    REQUIRE_FALSE(visibility.release_visible);
    REQUIRE_FALSE(visibility.remote_transport);
    REQUIRE(visibility.reason.find("process-local") != std::string::npos);
    REQUIRE(visibility.reason.find("remote cloud sync") != std::string::npos);
}

TEST_CASE("AISyncCoordinator syncs and restores history through local in-memory storage", "[ai][cloud]") {
    auto cloud = std::make_shared<urpg::social::LocalInMemoryCloudService>();
    const auto init = cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "");

    REQUIRE(init.success);
    REQUIRE(init.message.find("process-local memory") != std::string::npos);

    urpg::ai::AISyncCoordinator coordinator(cloud);

    auto source_chatbot = makeChatbot();
    source_chatbot.restoreHistory({{"system", "You are a village sage."},
                                   {"user", "Where is the shrine?"},
                                   {"assistant", "Beyond the cedar bridge."}});

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

TEST_CASE("AISyncCoordinator reports structured offline diagnostics", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    urpg::ai::AISyncCoordinator coordinator(cloud);

    auto chatbot = makeChatbot();
    chatbot.restoreHistory({{"user", "Hello"}});

    const auto sync = coordinator.syncHistoryToCloudDetailed("profile_alpha", chatbot);
    REQUIRE_FALSE(sync.success);
    REQUIRE(sync.code == urpg::ai::AISyncCoordinator::SyncErrorCode::Offline);
    REQUIRE(sync.message.find("offline") != std::string::npos);

    const auto restore = coordinator.restoreHistoryFromCloudDetailed("profile_alpha", chatbot);
    REQUIRE_FALSE(restore.success);
    REQUIRE(restore.code == urpg::ai::AISyncCoordinator::SyncErrorCode::Offline);
}

TEST_CASE("AISyncCoordinator reports cloud write failures", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);
    cloud->failWrites = true;

    urpg::ai::AISyncCoordinator coordinator(cloud);
    auto chatbot = makeChatbot();
    chatbot.restoreHistory({{"user", "Hello"}});

    const auto result = coordinator.syncHistoryToCloudDetailed("profile_alpha", chatbot);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::CloudWriteFailed);
    REQUIRE(result.key == "ai_history_profile_alpha.json");
    REQUIRE(result.message == "fake write rejected");
}

TEST_CASE("AISyncCoordinator reports missing AI history payloads", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);
    urpg::ai::AISyncCoordinator coordinator(cloud);
    auto chatbot = makeChatbot();

    const auto result = coordinator.restoreHistoryFromCloudDetailed("missing_profile", chatbot);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::MissingKey);
    REQUIRE(result.key == "ai_history_missing_profile.json");
}

TEST_CASE("AISyncCoordinator reports invalid JSON during restore", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);
    cloud->putText("ai_history_profile_alpha.json", "{not valid json");

    urpg::ai::AISyncCoordinator coordinator(cloud);
    auto chatbot = makeChatbot();

    const auto result = coordinator.restoreHistoryFromCloudDetailed("profile_alpha", chatbot);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::InvalidJson);
    REQUIRE(result.message.find("valid JSON") != std::string::npos);
}

TEST_CASE("AISyncCoordinator reports schema mismatches during restore", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);

    SECTION("root is not an array") {
        cloud->putText("ai_history_profile_alpha.json", R"({"role":"user","content":"hello"})");
        urpg::ai::AISyncCoordinator coordinator(cloud);
        auto chatbot = makeChatbot();

        const auto result = coordinator.restoreHistoryFromCloudDetailed("profile_alpha", chatbot);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::SchemaMismatch);
        REQUIRE(result.details.size() == 1);
        REQUIRE(result.details[0] == "root");
    }

    SECTION("message content is not a string") {
        cloud->putText("ai_history_profile_alpha.json", R"([{"role":"user","content":42}])");
        urpg::ai::AISyncCoordinator coordinator(cloud);
        auto chatbot = makeChatbot();

        const auto result = coordinator.restoreHistoryFromCloudDetailed("profile_alpha", chatbot);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::SchemaMismatch);
        REQUIRE(result.details.size() == 1);
        REQUIRE(result.details[0] == "[0].content");
    }
}

TEST_CASE("AISyncCoordinator checks project knowledge update feeds through cloud storage", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);
    cloud->putText("ai_knowledge_updates_project_alpha.json",
                   R"({"projectId":"project_alpha","updates":[{"id":"npc-lore-001","summary":"Shrine clue"}]})");

    urpg::ai::AISyncCoordinator coordinator(cloud);

    const auto result = coordinator.checkForRemoteKnowledgeUpdatesDetailed("project_alpha");
    REQUIRE(result.success);
    REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::None);
    REQUIRE(result.key == "ai_knowledge_updates_project_alpha.json");
    REQUIRE(result.details.size() == 1);
    REQUIRE(result.details[0] == "npc-lore-001");
    REQUIRE(coordinator.checkForRemoteKnowledgeUpdates("project_alpha"));
}

TEST_CASE("AISyncCoordinator reports missing and malformed knowledge update feeds", "[ai][cloud]") {
    auto cloud = std::make_shared<FakeCloudService>();
    REQUIRE(cloud->initialize(urpg::social::CloudProvider::LocalSimulated, "").success);

    SECTION("missing feed") {
        urpg::ai::AISyncCoordinator coordinator(cloud);

        const auto result = coordinator.checkForRemoteKnowledgeUpdatesDetailed("project_alpha");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::MissingKey);
        REQUIRE(result.key == "ai_knowledge_updates_project_alpha.json");
    }

    SECTION("invalid JSON") {
        cloud->putText("ai_knowledge_updates_project_alpha.json", "{not json");
        urpg::ai::AISyncCoordinator coordinator(cloud);

        const auto result = coordinator.checkForRemoteKnowledgeUpdatesDetailed("project_alpha");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::InvalidJson);
    }

    SECTION("project mismatch") {
        cloud->putText("ai_knowledge_updates_project_alpha.json", R"({"projectId":"other_project","updates":[]})");
        urpg::ai::AISyncCoordinator coordinator(cloud);

        const auto result = coordinator.checkForRemoteKnowledgeUpdatesDetailed("project_alpha");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::SchemaMismatch);
        REQUIRE(result.details.size() == 1);
        REQUIRE(result.details[0] == "projectId");
    }

    SECTION("invalid update entry") {
        cloud->putText("ai_knowledge_updates_project_alpha.json", R"({"projectId":"project_alpha","updates":[42]})");
        urpg::ai::AISyncCoordinator coordinator(cloud);

        const auto result = coordinator.checkForRemoteKnowledgeUpdatesDetailed("project_alpha");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.code == urpg::ai::AISyncCoordinator::SyncErrorCode::SchemaMismatch);
        REQUIRE(result.details.size() == 1);
        REQUIRE(result.details[0] == "updates[0]");
    }
}
