#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "analytics_event.h"

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

    uint64_t getSessionEventCount() const;
    nlohmann::json getBufferSnapshot() const;
    void resetSession();

private:
    bool m_optIn = false;
    uint64_t m_nextTick = 1;
    uint64_t m_sessionEventCount = 0;
    static constexpr size_t k_maxBufferSize = 1000;
    std::vector<AnalyticsEvent> m_buffer;
};

}  // namespace urpg::analytics
