#include "engine/core/analytics/analytics_uploader.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <unordered_map>

namespace urpg::analytics {

namespace {

std::string quoteCommandArg(const std::string& value) {
#ifdef _WIN32
    std::string quoted = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted += ch;
        }
    }
    quoted += "\"";
    return quoted;
#else
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
#endif
}

std::filesystem::path writeTempPayload(const std::string& payload) {
    const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("urpg_analytics_upload_" + unique + ".json");
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return {};
    }
    out << payload;
    return out.good() ? path : std::filesystem::path{};
}

} // namespace

void AnalyticsUploader::setUploadHandler(UploadHandler handler) {
    m_handler = std::move(handler);
    m_localJsonlExportPath.reset();
    m_httpEndpoint.reset();
    m_uploadMode = m_handler ? "custom_handler" : "disabled";
}

bool AnalyticsUploader::hasUploadHandler() const {
    return static_cast<bool>(m_handler);
}

void AnalyticsUploader::setBatchSize(size_t batchSize) {
    m_batchSize = batchSize;
}

void AnalyticsUploader::setLocalJsonlExportPath(std::filesystem::path path) {
    m_localJsonlExportPath = std::move(path);
    m_httpEndpoint.reset();
    m_uploadMode = "local_jsonl";
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

void AnalyticsUploader::setHttpJsonEndpoint(AnalyticsUploadEndpoint endpoint) {
    m_localJsonlExportPath.reset();
    m_httpEndpoint = std::move(endpoint);
    m_uploadMode = "http_json";
    const auto configured = *m_httpEndpoint;

    m_handler = [configured](const std::string& jsonPayload) {
        if (configured.url.empty()) {
            return false;
        }

        const auto payloadPath = writeTempPayload(jsonPayload);
        if (payloadPath.empty()) {
            return false;
        }

        std::ostringstream command;
        command << quoteCommandArg(configured.curlExecutable.empty() ? "curl" : configured.curlExecutable)
                << " -fsS -X POST"
                << " -H " << quoteCommandArg("Content-Type: application/json");
        for (const auto& [key, value] : configured.headers) {
            command << " -H " << quoteCommandArg(key + ": " + value);
        }
        if (!configured.bearerToken.empty()) {
            command << " -H " << quoteCommandArg("Authorization: Bearer " + configured.bearerToken);
        }
        command << " --data-binary " << quoteCommandArg("@" + payloadPath.string())
                << " " << quoteCommandArg(configured.url);
#ifdef _WIN32
        command << " >NUL 2>NUL";
#else
        command << " >/dev/null 2>/dev/null";
#endif

        const int exitCode = std::system(command.str().c_str());
        std::error_code ec;
        std::filesystem::remove(payloadPath, ec);
        return exitCode == 0;
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
