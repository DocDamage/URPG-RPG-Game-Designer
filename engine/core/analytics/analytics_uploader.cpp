#include "engine/core/analytics/analytics_uploader.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace urpg::analytics {

void AnalyticsUploader::setUploadHandler(UploadHandler handler) {
    m_handler = std::move(handler);
    m_localJsonlExportPath.reset();
}

bool AnalyticsUploader::hasUploadHandler() const {
    return static_cast<bool>(m_handler);
}

void AnalyticsUploader::setBatchSize(size_t batchSize) {
    m_batchSize = batchSize;
}

void AnalyticsUploader::setLocalJsonlExportPath(std::filesystem::path path) {
    m_localJsonlExportPath = std::move(path);
    const auto exportPath = *m_localJsonlExportPath;
    m_handler = [exportPath](const std::string& jsonPayload) {
        std::error_code ec;
        std::filesystem::create_directories(exportPath.parent_path(), ec);
        if (ec) {
            return false;
        }

        const auto batch = nlohmann::json::parse(jsonPayload, nullptr, false);
        if (batch.is_discarded()) {
            return false;
        }

        std::ofstream out(exportPath, std::ios::binary | std::ios::app);
        if (!out) {
            return false;
        }

        const nlohmann::json line = {
            {"schema", "urpg.analytics.local_export.v1"},
            {"transport", "local_jsonl"},
            {"batch", batch},
        };
        out << line.dump() << "\n";
        return out.good();
    };
}

UploadFlushResult AnalyticsUploader::flush(const std::vector<AnalyticsEvent>& events, const std::string& sessionId) {
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
            entry["category"] = event.category;
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
