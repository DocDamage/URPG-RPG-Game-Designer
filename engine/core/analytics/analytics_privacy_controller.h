#pragma once

#include "engine/core/analytics/analytics_event.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>

namespace urpg::analytics {

/**
 * @brief Consent state persisted and checked before any analytics activity.
 */
enum class ConsentState {
    Unknown,   ///< Not yet asked (default — analytics are suppressed)
    Granted,   ///< User opted in
    Denied,    ///< User opted out
};

/**
 * @brief Retention policy controlling how long events are kept before purge.
 */
struct RetentionPolicy {
    /** Maximum age of events to retain (in ticks).  0 = no age limit. */
    uint64_t maxAgeTicks = 0;
    /** Maximum total number of events to retain.  0 = no count limit. */
    size_t maxEventCount = 0;
};

/**
 * @brief Privacy controller: gates analytics collection behind explicit
 *        user consent, enforces retention limits, and provides data export
 *        and erasure workflows.
 *
 * Typical lifecycle:
 *  1. On first launch: call requestConsent() / recordConsentDecision() once
 *     the player responds to the consent prompt.
 *  2. On every session start: call isAnalyticsPermitted() before dispatching.
 *  3. Periodically: call applyRetentionPolicy() to purge stale events.
 *  4. On user request: call exportUserData() or eraseUserData().
 */
class AnalyticsPrivacyController {
public:
    // ── Consent management ────────────────────────────────────────────────────

    /** @brief Record the player's consent decision. */
    void recordConsentDecision(ConsentState state);

    /** @brief Current consent state. */
    ConsentState getConsentState() const;

    /**
     * @brief Returns true iff analytics may be collected and dispatched.
     *
     * Analytics are permitted only when consent is Granted.
     */
    bool isAnalyticsPermitted() const;

    // ── Retention management ──────────────────────────────────────────────────

    /**
     * @brief Configure the active retention policy.
     *
     * Call applyRetentionPolicy() to enforce it against a live event buffer.
     */
    void setRetentionPolicy(const RetentionPolicy& policy);

    const RetentionPolicy& getRetentionPolicy() const;

    /**
     * @brief Purge events from @p events that violate the current retention
     *        policy (by age or count).
     *
     * Events are assumed to be ordered oldest-first (ascending timestamp).
     * Excess events are removed from the front.
     *
     * @return  The number of events removed.
     */
    size_t applyRetentionPolicy(std::vector<AnalyticsEvent>& events,
                                uint64_t currentTick) const;

    // ── Data export and erasure ───────────────────────────────────────────────

    /**
     * @brief Produce a privacy-safe JSON export of @p events suitable for
     *        presenting to the player in response to a data-access request.
     *
     * PII-like parameter keys listed in @p piiKeys are redacted from the
     * export.  The export includes consent state and retention policy metadata.
     */
    nlohmann::json exportUserData(const std::vector<AnalyticsEvent>& events,
                                  const std::vector<std::string>& piiKeys = {}) const;

    /**
     * @brief Clear @p events in place (full erasure on user request).
     *
     * After this call the caller is responsible for also resetting the
     * dispatcher's buffer and uploader aggregate.
     *
     * @return  Number of events erased.
     */
    size_t eraseUserData(std::vector<AnalyticsEvent>& events) const;

private:
    ConsentState m_consent = ConsentState::Unknown;
    RetentionPolicy m_retention;

    static std::string consentToString(ConsentState state);
};

} // namespace urpg::analytics
