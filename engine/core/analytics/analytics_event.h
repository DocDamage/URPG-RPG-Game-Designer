#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::analytics {

struct AnalyticsEvent {
    std::string eventName;
    std::string category;
    uint64_t timestamp = 0;  // deterministic tick count
    std::map<std::string, std::string> parameters;
};

class AnalyticsEventBuffer {
public:
    void pushEvent(const AnalyticsEvent& event);
    std::vector<AnalyticsEvent> flush();
    size_t size() const;
    void clear();
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);

private:
    std::vector<AnalyticsEvent> m_events;
};

}  // namespace urpg::analytics
