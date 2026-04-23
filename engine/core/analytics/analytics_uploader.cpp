#include "engine/core/analytics/analytics_uploader.h"

#include <nlohmann/json.hpp>
#include <unordered_map>

namespace urpg::analytics {

void AnalyticsUploader::setUploadHandler(UploadHandler handler) {
    m_handler = std::move(handler);
}

void AnalyticsUploader::setBatchSize(size_t batchSize) {
    m_batchSize = batchSize;
}

UploadFlushResult AnalyticsUploader::flush(const std::vector<AnalyticsEvent>& events,
                                           const std::string& sessionId) {
    UploadFlushResult result;

    // Accumulate session aggregates regardless of upload availability.
    ++m_aggregate.totalSessions;
    m_aggregate.totalEvents += events.size();
    for (const auto& event : events) {
        m_aggregate.countsByCategory[event.category]++;
        m_aggregate.countsByEvent[event.eventName]++;
    }

    if (!m_handler || events.empty()) {
        result.success = true;
        result.eventsFlushed = 0;
        return result;
    }

    // Batch and upload.
    const size_t batchSize = (m_batchSize == 0) ? events.size() : m_batchSize;
    size_t totalFlushed = 0;

    for (size_t offset = 0; offset < events.size(); offset += batchSize) {
        const size_t end = std::min(offset + batchSize, events.size());

        nlohmann::json batch = nlohmann::json::array();
        for (size_t i = offset; i < end; ++i) {
            const auto& event = events[i];
            nlohmann::json entry;
            entry["eventName"] = event.eventName;
            entry["category"]  = event.category;
            entry["timestamp"] = event.timestamp;
            if (!sessionId.empty()) {
                entry["sessionId"] = sessionId;
            }
            nlohmann::json params = nlohmann::json::object();
            for (const auto& [k, v] : event.parameters) {
                params[k] = v;
            }
            entry["parameters"] = params;
            batch.push_back(std::move(entry));
        }

        const std::string payload = batch.dump();
        if (!m_handler(payload)) {
            result.success = false;
            result.eventsFlushed = totalFlushed;
            result.errorMessage = "Upload handler returned failure";
            return result;
        }

        totalFlushed += (end - offset);
    }

    result.success = true;
    result.eventsFlushed = totalFlushed;
    return result;
}

const SessionAggregate& AnalyticsUploader::getAggregate() const {
    return m_aggregate;
}

void AnalyticsUploader::resetAggregate() {
    m_aggregate = SessionAggregate{};
}

} // namespace urpg::analytics
