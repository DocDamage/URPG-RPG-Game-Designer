#pragma once

#include "engine/core/analytics/analytics_event.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::analytics {

/**
 * @brief Result of an analytics upload flush.
 */
struct UploadFlushResult {
    bool success = false;
    size_t eventsFlushed = 0;
    std::string errorMessage;
};

/**
 * @brief Summary of aggregated session analytics.
 *
 * Produced by aggregateSession() for structured diagnostics.
 */
struct SessionAggregate {
    uint64_t totalEvents = 0;
    uint64_t totalSessions = 0;
    /** Map of category name → event count across all sessions. */
    std::unordered_map<std::string, uint64_t> countsByCategory;
    /** Map of event name → count across all sessions. */
    std::unordered_map<std::string, uint64_t> countsByEvent;
};

/**
 * @brief Provides upload batching, session aggregation, and flush mechanics for
 *        the AnalyticsDispatcher event buffer.
 *
 * The AnalyticsUploader is intentionally transport-agnostic: callers supply an
 * upload backend via setUploadHandler().  In production this would be a
 * network-transport lambda; in tests it can be an in-memory accumulator.
 *
 * Session aggregation is purely in-memory and deterministic — it accumulates
 * statistics from every batch passed to flush() so that editors and diagnostics
 * panels can inspect cross-session event distributions without raw PII.
 */
class AnalyticsUploader {
  public:
    /**
     * @brief Upload-backend callback type.
     *
     * Receives the serialised batch as a JSON string.  Returns true on
     * success, false on failure.  Must not throw.
     */
    using UploadHandler = std::function<bool(const std::string& jsonPayload)>;

    /**
     * @brief Bind the upload backend.  Pass nullptr to disable uploads while
     *        still accumulating session aggregates.
     */
    void setUploadHandler(UploadHandler handler);
    bool hasUploadHandler() const;

    /**
     * @brief Set the maximum number of events per flush batch.  Defaults to
     *        100.  A value of 0 means flush all pending events in one batch.
     */
    void setBatchSize(size_t batchSize);

    /**
     * @brief Flush @p events to the upload backend and accumulate session
     *        aggregates.  Events are consumed from the provided snapshot — the
     *        dispatcher's own buffer is NOT modified by this call.
     *
     * @param events    Snapshot of events to upload (e.g. from
     *                  AnalyticsDispatcher::snapshotEvents()).
     * @param sessionId Opaque session identifier attached to every batch.
     * @return          UploadFlushResult with success flag and event count.
     */
    UploadFlushResult flush(const std::vector<AnalyticsEvent>& events, const std::string& sessionId = "");

    /**
     * @brief Return the current cross-session aggregate.
     */
    const SessionAggregate& getAggregate() const;

    /**
     * @brief Reset the in-memory aggregate (e.g. on new release build).
     */
    void resetAggregate();

  private:
    UploadHandler m_handler;
    size_t m_batchSize = 100;
    SessionAggregate m_aggregate;
};

} // namespace urpg::analytics
