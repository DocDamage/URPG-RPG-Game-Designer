#include "analytics_panel.h"

namespace urpg::editor {

void AnalyticsPanel::bindDispatcher(urpg::analytics::AnalyticsDispatcher* dispatcher) {
    m_dispatcher = dispatcher;
}

void AnalyticsPanel::render() {
    nlohmann::json snapshot;

    if (m_dispatcher == nullptr) {
        snapshot["optIn"] = false;
        snapshot["sessionEventCount"] = 0;
        snapshot["recentEvents"] = nlohmann::json::array();
        m_lastSnapshot = std::move(snapshot);
        return;
    }

    snapshot["optIn"] = m_dispatcher->isOptIn();
    snapshot["sessionEventCount"] = m_dispatcher->getSessionEventCount();

    nlohmann::json allEvents = m_dispatcher->getBufferSnapshot();
    nlohmann::json recentEvents = nlohmann::json::array();

    const size_t sampleSize = 10;
    size_t start = 0;
    if (allEvents.size() > sampleSize) {
        start = allEvents.size() - sampleSize;
    }

    for (size_t i = start; i < allEvents.size(); ++i) {
        recentEvents.push_back(allEvents[i]);
    }

    snapshot["recentEvents"] = std::move(recentEvents);
    m_lastSnapshot = std::move(snapshot);
}

nlohmann::json AnalyticsPanel::lastRenderSnapshot() const {
    return m_lastSnapshot;
}

}  // namespace urpg::editor
