#include "analytics_dispatcher.h"

namespace urpg::analytics {

void AnalyticsDispatcher::setOptIn(bool enabled) {
    m_optIn = enabled;
}

bool AnalyticsDispatcher::isOptIn() const {
    return m_optIn;
}

void AnalyticsDispatcher::dispatchEvent(const std::string& eventName, const std::string& category,
                                        const std::map<std::string, std::string>& params) {
    if (!m_optIn) {
        return;
    }

    AnalyticsEvent event;
    event.eventName = eventName;
    event.category = category;
    event.timestamp = m_nextTick++;
    event.parameters = params;

    m_buffer.push_back(std::move(event));
    ++m_sessionEventCount;

    if (m_buffer.size() > k_maxBufferSize) {
        m_buffer.erase(m_buffer.begin());
    }
}

void AnalyticsDispatcher::dispatchEvent(const std::string& eventName, const std::string& category) {
    dispatchEvent(eventName, category, {});
}

void AnalyticsDispatcher::setAllowedCategories(const std::vector<std::string>& allowedCategories) {
    m_allowedCategories = allowedCategories;
}

std::vector<std::string> AnalyticsDispatcher::getAllowedCategories() const {
    return m_allowedCategories;
}

uint64_t AnalyticsDispatcher::getSessionEventCount() const {
    return m_sessionEventCount;
}

std::vector<AnalyticsEvent> AnalyticsDispatcher::snapshotEvents() const {
    return m_buffer;
}

std::vector<AnalyticsValidationIssue> AnalyticsDispatcher::getValidationIssues() const {
    AnalyticsDispatcherValidator validator;
    AnalyticsValidationRules rules;
    rules.allowedCategories = m_allowedCategories;
    return validator.validate(m_buffer, rules);
}

nlohmann::json AnalyticsDispatcher::getBufferSnapshot() const {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& event : m_buffer) {
        nlohmann::json entry;
        entry["eventName"] = event.eventName;
        entry["category"] = event.category;
        entry["timestamp"] = event.timestamp;
        entry["parameters"] = event.parameters;
        result.push_back(std::move(entry));
    }
    return result;
}

size_t AnalyticsDispatcher::getQueuedEventCount() const {
    return m_buffer.size();
}

size_t AnalyticsDispatcher::clearQueuedEvents() {
    const size_t count = m_buffer.size();
    m_buffer.clear();
    return count;
}

void AnalyticsDispatcher::resetSession() {
    m_buffer.clear();
    m_nextTick = 1;
    m_sessionEventCount = 0;
}

} // namespace urpg::analytics
