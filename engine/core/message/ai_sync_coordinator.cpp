#include "ai_sync_coordinator.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace urpg::ai {

AISyncCoordinator::AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService)
    : m_cloud(cloudService) {}

namespace {

AISyncCoordinator::SyncResult makeFailure(AISyncCoordinator::SyncErrorCode code, std::string message,
                                          std::string key = {}) {
    AISyncCoordinator::SyncResult result;
    result.success = false;
    result.code = code;
    result.message = std::move(message);
    result.key = std::move(key);
    return result;
}

AISyncCoordinator::SyncResult makeSuccess(std::string message, std::string key = {}) {
    AISyncCoordinator::SyncResult result;
    result.success = true;
    result.code = AISyncCoordinator::SyncErrorCode::None;
    result.message = std::move(message);
    result.key = std::move(key);
    return result;
}

} // namespace

std::string AISyncCoordinator::historyKeyForProfile(const std::string& profileId) {
    return "ai_history_" + profileId + ".json";
}

std::string AISyncCoordinator::knowledgeUpdatesKeyForProject(const std::string& projectId) {
    return "ai_knowledge_updates_" + projectId + ".json";
}

bool AISyncCoordinator::syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot) {
    return syncHistoryToCloudDetailed(profileId, chatbot).success;
}

AISyncCoordinator::SyncResult AISyncCoordinator::syncHistoryToCloudDetailed(const std::string& profileId,
                                                                            const ChatbotComponent& chatbot) {
    // NOT LIVE with the in-tree LocalInMemoryCloudService:
    // this serializes history and hands it to whichever ICloudService is injected,
    // but the default in-tree implementation only stores bytes in process-local memory.
    if (!m_cloud || !m_cloud->isOnline()) {
        m_lastResult =
            makeFailure(SyncErrorCode::Offline, "Cloud service is offline or unavailable for AI history sync.");
        return m_lastResult;
    }

    nlohmann::json history = nlohmann::json::array();
    const auto& chatHistory = chatbot.getHistory(); // Assuming getHistory() visibility

    for (const auto& msg : chatHistory) {
        history.push_back({{"role", msg.role}, {"content", msg.content}});
    }

    std::string serialized = history.dump();
    std::vector<uint8_t> data(serialized.begin(), serialized.end());

    std::string key = historyKeyForProfile(profileId);
    auto result = m_cloud->syncToCloud(key, data);

    if (!result.success) {
        m_lastResult =
            makeFailure(SyncErrorCode::CloudWriteFailed,
                        result.message.empty() ? "Cloud service rejected AI history sync." : result.message, key);
        return m_lastResult;
    }

    m_lastResult = makeSuccess(result.message.empty() ? "AI history synced." : result.message, key);
    return m_lastResult;
}

bool AISyncCoordinator::restoreHistoryFromCloud(const std::string& profileId, ChatbotComponent& chatbot) {
    return restoreHistoryFromCloudDetailed(profileId, chatbot).success;
}

AISyncCoordinator::SyncResult AISyncCoordinator::restoreHistoryFromCloudDetailed(const std::string& profileId,
                                                                                 ChatbotComponent& chatbot) {
    // NOT LIVE with the in-tree LocalInMemoryCloudService:
    // restores only from current process-local memory and does not prove any
    // cross-device or remote-cloud transport path.
    if (!m_cloud || !m_cloud->isOnline()) {
        m_lastResult =
            makeFailure(SyncErrorCode::Offline, "Cloud service is offline or unavailable for AI history restore.");
        return m_lastResult;
    }

    std::string key = historyKeyForProfile(profileId);
    std::vector<uint8_t> data = m_cloud->fetchFromCloud(key);

    if (data.empty()) {
        m_lastResult =
            makeFailure(SyncErrorCode::MissingKey, "No AI history payload found for profile: " + profileId, key);
        return m_lastResult;
    }

    std::string serialized(data.begin(), data.end());
    nlohmann::json history = nlohmann::json::parse(serialized, nullptr, false);
    if (history.is_discarded()) {
        m_lastResult = makeFailure(SyncErrorCode::InvalidJson, "AI history payload is not valid JSON.", key);
        return m_lastResult;
    }
    if (!history.is_array()) {
        m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                   "AI history payload schema mismatch: root must be an array.", key);
        m_lastResult.details.push_back("root");
        return m_lastResult;
    }

    std::vector<ChatMessage> restoredHistory;
    for (size_t index = 0; index < history.size(); ++index) {
        const auto& item = history[index];
        if (!item.is_object()) {
            m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                       "AI history payload schema mismatch: message entry must be an object.", key);
            m_lastResult.details.push_back("[" + std::to_string(index) + "]");
            return m_lastResult;
        }
        if (!item.contains("role") || !item["role"].is_string()) {
            m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                       "AI history payload schema mismatch: message role must be a string.", key);
            m_lastResult.details.push_back("[" + std::to_string(index) + "].role");
            return m_lastResult;
        }
        if (!item.contains("content") || !item["content"].is_string()) {
            m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                       "AI history payload schema mismatch: message content must be a string.", key);
            m_lastResult.details.push_back("[" + std::to_string(index) + "].content");
            return m_lastResult;
        }

        restoredHistory.push_back({item["role"].get<std::string>(), item["content"].get<std::string>()});
    }

    try {
        chatbot.restoreHistory(restoredHistory); // Assuming restoreHistory() visibility
    } catch (const std::exception& e) {
        m_lastResult = makeFailure(SyncErrorCode::RestoreFailed,
                                   std::string("Failed to restore AI history into chatbot: ") + e.what(), key);
        return m_lastResult;
    } catch (...) {
        m_lastResult = makeFailure(SyncErrorCode::RestoreFailed,
                                   "Failed to restore AI history into chatbot: unknown exception.", key);
        return m_lastResult;
    }

    m_lastResult = makeSuccess("AI history restored.", key);
    return m_lastResult;
}

bool AISyncCoordinator::checkForRemoteKnowledgeUpdates(const std::string& projectId) {
    return checkForRemoteKnowledgeUpdatesDetailed(projectId).success;
}

AISyncCoordinator::SyncResult AISyncCoordinator::checkForRemoteKnowledgeUpdatesDetailed(const std::string& projectId) {
    // NOT LIVE with the in-tree LocalInMemoryCloudService:
    // this checks whichever ICloudService is injected. The shipped backend only
    // reads process-local memory, while out-of-tree providers can expose the same key contract.
    if (!m_cloud || !m_cloud->isOnline()) {
        m_lastResult =
            makeFailure(SyncErrorCode::Offline, "Cloud service is offline or unavailable for AI knowledge updates.");
        return m_lastResult;
    }

    const std::string key = knowledgeUpdatesKeyForProject(projectId);
    const std::vector<uint8_t> data = m_cloud->fetchFromCloud(key);
    if (data.empty()) {
        m_lastResult =
            makeFailure(SyncErrorCode::MissingKey, "No AI knowledge update feed found for project: " + projectId, key);
        return m_lastResult;
    }

    const std::string serialized(data.begin(), data.end());
    const nlohmann::json feed = nlohmann::json::parse(serialized, nullptr, false);
    if (feed.is_discarded()) {
        m_lastResult = makeFailure(SyncErrorCode::InvalidJson, "AI knowledge update feed is not valid JSON.", key);
        return m_lastResult;
    }
    if (!feed.is_object()) {
        m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                   "AI knowledge update feed schema mismatch: root must be an object.", key);
        m_lastResult.details.push_back("root");
        return m_lastResult;
    }
    if (!feed.contains("projectId") || !feed["projectId"].is_string()) {
        m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                   "AI knowledge update feed schema mismatch: projectId must be a string.", key);
        m_lastResult.details.push_back("projectId");
        return m_lastResult;
    }
    if (feed["projectId"].get<std::string>() != projectId) {
        m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                   "AI knowledge update feed projectId does not match requested project.", key);
        m_lastResult.details.push_back("projectId");
        return m_lastResult;
    }
    if (!feed.contains("updates") || !feed["updates"].is_array()) {
        m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                   "AI knowledge update feed schema mismatch: updates must be an array.", key);
        m_lastResult.details.push_back("updates");
        return m_lastResult;
    }

    const auto& updates = feed["updates"];
    std::vector<std::string> updateDetails;
    for (size_t index = 0; index < updates.size(); ++index) {
        const auto& update = updates[index];
        if (!update.is_object()) {
            m_lastResult = makeFailure(SyncErrorCode::SchemaMismatch,
                                       "AI knowledge update feed schema mismatch: update entry must be an object.", key);
            m_lastResult.details.push_back("updates[" + std::to_string(index) + "]");
            return m_lastResult;
        }
        if (update.contains("id")) {
            if (!update["id"].is_string()) {
                m_lastResult =
                    makeFailure(SyncErrorCode::SchemaMismatch,
                                "AI knowledge update feed schema mismatch: update id must be a string.", key);
                m_lastResult.details.push_back("updates[" + std::to_string(index) + "].id");
                return m_lastResult;
            }
            updateDetails.push_back(update["id"].get<std::string>());
        } else {
            updateDetails.push_back("updates[" + std::to_string(index) + "]");
        }
    }

    m_lastResult = makeSuccess("AI knowledge update feed available: " + std::to_string(updates.size()) + " update(s).",
                               key);
    m_lastResult.details = std::move(updateDetails);
    return m_lastResult;
}

AISyncCoordinator::SyncResult AISyncCoordinator::lastResult() const {
    return m_lastResult;
}

} // namespace urpg::ai
