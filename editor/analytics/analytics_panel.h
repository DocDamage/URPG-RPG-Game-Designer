#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_endpoint_profile.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"

namespace urpg::editor {

class AnalyticsPanel {
  public:
    void bindDispatcher(urpg::analytics::AnalyticsDispatcher* dispatcher);
    void bindUploader(urpg::analytics::AnalyticsUploader* uploader);
    void bindPrivacyController(urpg::analytics::AnalyticsPrivacyController* privacyController);
    void bindEndpointProfile(const urpg::analytics::AnalyticsEndpointProfile* endpointProfile);
    void setSessionId(std::string sessionId);
    void render();
    nlohmann::json lastRenderSnapshot() const;

    bool setOptIn(bool enabled);
    bool applyEndpointProfile();
    size_t clearQueuedEvents();
    bool flushQueuedEvents();
    void clearLastAction();

  private:
    void rebuildSnapshot();
    void recordAction(std::string action, bool success, std::string message, size_t affectedEvents = 0);
    static std::string consentStateToString(urpg::analytics::ConsentState state);

    urpg::analytics::AnalyticsDispatcher* m_dispatcher = nullptr;
    urpg::analytics::AnalyticsUploader* m_uploader = nullptr;
    urpg::analytics::AnalyticsPrivacyController* m_privacyController = nullptr;
    const urpg::analytics::AnalyticsEndpointProfile* m_endpointProfile = nullptr;
    std::string m_sessionId = "editor_session";
    nlohmann::json m_lastSnapshot;
    nlohmann::json m_lastAction;
};

} // namespace urpg::editor
