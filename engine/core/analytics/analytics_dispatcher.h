#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "analytics_event.h"
#include "analytics_dispatcher_validator.h"

namespace urpg::analytics {

class AnalyticsDispatcher {
public:
    void setOptIn(bool enabled);
    bool isOptIn() const;

    void dispatchEvent(
        const std::string& eventName,
        const std::string& category,
        const std::map<std::string, std::string>& params);

    void dispatchEvent(
        const std::string& eventName,
        const std::string& category);

    void setAllowedCategories(const std::vector<std::string>& allowedCategories);
    std::vector<std::string> getAllowedCategories() const;
    uint64_t getSessionEventCount() const;
    std::vector<AnalyticsEvent> snapshotEvents() const;
    std::vector<AnalyticsValidationIssue> getValidationIssues() const;
    nlohmann::json getBufferSnapshot() const;
    void resetSession();

private:
    bool m_optIn = false;
    uint64_t m_nextTick = 1;
    uint64_t m_sessionEventCount = 0;
    static constexpr size_t k_maxBufferSize = 1000;
    std::vector<std::string> m_allowedCategories;
    std::vector<AnalyticsEvent> m_buffer;
};

}  // namespace urpg::analytics
