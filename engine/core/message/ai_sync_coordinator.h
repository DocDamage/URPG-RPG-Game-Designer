#pragma once

#include "engine/core/social/cloud_service.h"
#include "engine/core/message/chatbot_component.h"
#include <memory>
#include <string>

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
    AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService);
    virtual ~AISyncCoordinator() = default;

    /**
     * @brief Pushes current chatbot history to the configured cloud-service backend.
     */
    bool syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot);

    /**
     * @brief Pulls chatbot history from the configured cloud-service backend and restores it.
     */
    bool restoreHistoryFromCloud(const std::string& profileId, ChatbotComponent& chatbot);

    /**
     * @brief Placeholder hook for checking remote AI knowledge updates.
     */
    bool checkForRemoteKnowledgeUpdates(const std::string& projectId);

private:
    std::shared_ptr<urpg::social::ICloudService> m_cloud;
};

} // namespace urpg::ai
