#include "ai_sync_coordinator.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace urpg::ai {

AISyncCoordinator::AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService)
    : m_cloud(cloudService) {}

bool AISyncCoordinator::syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot) {
    // NOT LIVE with the in-tree LocalInMemoryCloudService:
    // this serializes history and hands it to whichever ICloudService is injected,
    // but the default in-tree implementation only stores bytes in process-local memory.
    if (!m_cloud || !m_cloud->isOnline()) return false;

    nlohmann::json history = nlohmann::json::array();
    const auto& chatHistory = chatbot.getHistory(); // Assuming getHistory() visibility

    for (const auto& msg : chatHistory) {
        history.push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }

    std::string serialized = history.dump();
    std::vector<uint8_t> data(serialized.begin(), serialized.end());
    
    std::string key = "ai_history_" + profileId + ".json";
    auto result = m_cloud->syncToCloud(key, data);
    
    return result.success;
}

bool AISyncCoordinator::restoreHistoryFromCloud(const std::string& profileId, ChatbotComponent& chatbot) {
    // NOT LIVE with the in-tree LocalInMemoryCloudService:
    // restores only from current process-local memory and does not prove any
    // cross-device or remote-cloud transport path.
    if (!m_cloud || !m_cloud->isOnline()) return false;

    std::string key = "ai_history_" + profileId + ".json";
    std::vector<uint8_t> data = m_cloud->fetchFromCloud(key);
    
    if (data.empty()) return false;

    std::string serialized(data.begin(), data.end());
    try {
        nlohmann::json history = nlohmann::json::parse(serialized);
        if (!history.is_array()) return false;

        std::vector<ChatMessage> restoredHistory;
        for (const auto& item : history) {
            restoredHistory.push_back({
                item.value("role", "user"),
                item.value("content", "")
            });
        }

        chatbot.restoreHistory(restoredHistory); // Assuming restoreHistory() visibility
        return true;
    } catch (...) {
        return false;
    }
}

bool AISyncCoordinator::checkForRemoteKnowledgeUpdates(const std::string& projectId) {
    static_cast<void>(projectId);

    // NOT LIVE: there is no remote transport or provider-specific knowledge feed in-tree.
    // Return false so callers do not mistake this placeholder for a successful remote check.
    return false;
}

} // namespace urpg::ai
