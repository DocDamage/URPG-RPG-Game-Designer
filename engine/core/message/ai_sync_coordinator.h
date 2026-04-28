#pragma once

#include "engine/core/message/chatbot_component.h"
#include "engine/core/social/cloud_service.h"
#include <memory>
#include <string>
#include <vector>

namespace urpg::ai {

/**
 * @brief Coordinates AI history serialization through an injected cloud-service abstraction.
 * The accepted in-tree contract is process-local only: cross-device persistence
 * depends on an out-of-tree ICloudService implementation, and the shipped
 * LocalInMemoryCloudService remains harness/test behavior rather than a live
 * cloud sync path.
 */
class AISyncCoordinator {
  public:
    enum class SyncErrorCode {
        None,
        Offline,
        MissingKey,
        CloudWriteFailed,
        InvalidJson,
        SchemaMismatch,
        RestoreFailed,
        RemoteUpdatesUnsupported
    };

    struct SyncResult {
        bool success = false;
        SyncErrorCode code = SyncErrorCode::None;
        std::string message;
        std::string key;
        std::vector<std::string> details;
    };

    AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService);
    virtual ~AISyncCoordinator() = default;

    /**
     * @brief Pushes current chatbot history to the configured cloud-service backend.
     */
    bool syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot);
    SyncResult syncHistoryToCloudDetailed(const std::string& profileId, const ChatbotComponent& chatbot);

    /**
     * @brief Pulls chatbot history from the configured cloud-service backend and restores it.
     */
    bool restoreHistoryFromCloud(const std::string& profileId, ChatbotComponent& chatbot);
    SyncResult restoreHistoryFromCloudDetailed(const std::string& profileId, ChatbotComponent& chatbot);

    /**
     * @brief Checks the configured cloud-service backend for a project knowledge update feed.
     */
    bool checkForRemoteKnowledgeUpdates(const std::string& projectId);
    SyncResult checkForRemoteKnowledgeUpdatesDetailed(const std::string& projectId);

    SyncResult lastResult() const;

  private:
    static std::string historyKeyForProfile(const std::string& profileId);
    static std::string knowledgeUpdatesKeyForProject(const std::string& projectId);

    std::shared_ptr<urpg::social::ICloudService> m_cloud;
    SyncResult m_lastResult;
};

} // namespace urpg::ai
