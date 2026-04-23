#include "engine/core/analytics/analytics_privacy_controller.h"

#include <algorithm>

namespace urpg::analytics {

void AnalyticsPrivacyController::recordConsentDecision(ConsentState state) {
    m_consent = state;
}

ConsentState AnalyticsPrivacyController::getConsentState() const {
    return m_consent;
}

bool AnalyticsPrivacyController::isAnalyticsPermitted() const {
    return m_consent == ConsentState::Granted;
}

void AnalyticsPrivacyController::setRetentionPolicy(const RetentionPolicy& policy) {
    m_retention = policy;
}

const RetentionPolicy& AnalyticsPrivacyController::getRetentionPolicy() const {
    return m_retention;
}

size_t AnalyticsPrivacyController::applyRetentionPolicy(
    std::vector<AnalyticsEvent>& events, uint64_t currentTick) const {
    size_t removed = 0;

    // Age-based purge (oldest-first assumption)
    if (m_retention.maxAgeTicks > 0) {
        auto it = events.begin();
        while (it != events.end() && currentTick >= it->timestamp &&
               (currentTick - it->timestamp) > m_retention.maxAgeTicks) {
            ++it;
            ++removed;
        }
        events.erase(events.begin(), it);
    }

    // Count-based purge (remove oldest from front)
    if (m_retention.maxEventCount > 0 && events.size() > m_retention.maxEventCount) {
        const size_t excess = events.size() - m_retention.maxEventCount;
        events.erase(events.begin(), events.begin() + static_cast<std::ptrdiff_t>(excess));
        removed += excess;
    }

    return removed;
}

nlohmann::json AnalyticsPrivacyController::exportUserData(
    const std::vector<AnalyticsEvent>& events,
    const std::vector<std::string>& piiKeys) const {
    nlohmann::json doc;
    doc["consentState"]       = consentToString(m_consent);
    doc["retentionMaxAgeTicks"]   = m_retention.maxAgeTicks;
    doc["retentionMaxEventCount"] = m_retention.maxEventCount;
    doc["eventCount"] = events.size();

    nlohmann::json eventsArray = nlohmann::json::array();
    for (const auto& event : events) {
        nlohmann::json entry;
        entry["eventName"] = event.eventName;
        entry["category"]  = event.category;
        entry["timestamp"] = event.timestamp;
        nlohmann::json params = nlohmann::json::object();
        for (const auto& [k, v] : event.parameters) {
            bool isPii = std::find(piiKeys.begin(), piiKeys.end(), k) != piiKeys.end();
            params[k] = isPii ? "[REDACTED]" : v;
        }
        entry["parameters"] = params;
        eventsArray.push_back(std::move(entry));
    }
    doc["events"] = eventsArray;
    return doc;
}

size_t AnalyticsPrivacyController::eraseUserData(std::vector<AnalyticsEvent>& events) const {
    const size_t count = events.size();
    events.clear();
    return count;
}

std::string AnalyticsPrivacyController::consentToString(ConsentState state) {
    switch (state) {
        case ConsentState::Granted: return "Granted";
        case ConsentState::Denied:  return "Denied";
        default:                    return "Unknown";
    }
}

} // namespace urpg::analytics
