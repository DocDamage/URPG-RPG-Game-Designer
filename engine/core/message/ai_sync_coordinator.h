#pragma once

#include "engine/core/social/cloud_service.h"
#include "engine/core/message/chatbot_component.h"
#include <memory>
#include <string>

namespace urpg::ai {

/**
 * @brief Coordinates the synchronization of AI conversation history and knowledge bases
 * across cloud providers (Steam, GOG, Epic, or Generic HTTP).
 * This ensures the 'Game Guide' remembers your questions even if you switch devices.
 */
class AISyncCoordinator {
public:
    AISyncCoordinator(std::shared_ptr<urpg::social::ICloudService> cloudService);
    virtual ~AISyncCoordinator() = default;

    /**
     * @brief Pushes current chatbot history to the cloud.
     */
    bool syncHistoryToCloud(const std::string& profileId, const ChatbotComponent& chatbot);

    /**
     * @brief Pulls chatbot history from the cloud and restores it.
     */
    bool restoreHistoryFromCloud(const std::string& profileId, ChatbotComponent& chatbot);

    /**
     * @brief Checks if there are newer AI insights available in the cloud (e.g., from community wiki mirrors).
     */
    bool checkForRemoteKnowledgeUpdates(const std::string& projectId);

private:
    std::shared_ptr<urpg::social::ICloudService> m_cloud;
};

} // namespace urpg::ai