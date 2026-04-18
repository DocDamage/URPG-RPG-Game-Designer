#include "ai_sync_coordinator.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace urpg::ai {

AISyncCoordinator::AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService)
    : m_cloud(cloudService) {}

bool AISyncCoordinator::syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot) {
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
    // Phase 4 development: Placeholder for dynamically updating the 'Game Guide'
    // with community-driven wiki data or developer-pushed story corrections.
    return true;
}

} // namespace urpg::ai