#include "engine/core/privacy/privacy_network_policy.h"

#include <algorithm>

namespace urpg::privacy {

const char* networkPurposeName(const NetworkPurpose purpose) {
    switch (purpose) {
    case NetworkPurpose::Telemetry: return "telemetry";
    case NetworkPurpose::SupportUpload: return "support_upload";
    case NetworkPurpose::AiProvider: return "ai_provider";
    case NetworkPurpose::UpdateCheck: return "update_check";
    case NetworkPurpose::Multiplayer: return "multiplayer";
    }
    return "unknown";
}

PrivacyNetworkGate::PrivacyNetworkGate(PrivacyPolicy policy) : policy_(policy) {
    if (policy_.maximum_retention_days == 0) policy_.retention_configured = false;
}

NetworkDecision PrivacyNetworkGate::authorize(const NetworkRequest& request) {
    NetworkDecision decision{request.id, request.purpose, false, {}, {}};
    if (request.id.empty() || request.endpoint_profile_id.empty()) {
        decision.code = "network_request_invalid";
        decision.message = "Network requests require stable request and reviewed endpoint profile IDs.";
    } else if (!request.user_initiated && (policy_.offline_first || !policy_.automatic_upload)) {
        decision.code = "network_automatic_request_blocked";
        decision.message = "Offline-first defaults block automatic outbound requests.";
    } else if (policy_.require_preview && !request.preview_approved) {
        decision.code = "network_preview_required";
        decision.message = "Review the outbound payload preview before sending.";
    } else if (policy_.require_explicit_consent && !request.explicit_consent) {
        decision.code = "network_consent_required";
        decision.message = "Explicit consent is required for outbound requests.";
    } else if (request.purpose == NetworkPurpose::Telemetry && policy_.telemetry_opt_in && !request.explicit_consent) {
        decision.code = "network_telemetry_opt_in_required";
        decision.message = "Telemetry remains disabled until opt-in.";
    } else {
        decision.allowed = true;
        decision.code = "network_request_authorized";
        decision.message = "Reviewed user-initiated outbound request authorized.";
    }
    ledger_.push_back(decision);
    return decision;
}

size_t PrivacyNetworkGate::allowedCount() const {
    return static_cast<size_t>(std::count_if(ledger_.begin(), ledger_.end(), [](const auto& row) { return row.allowed; }));
}

size_t PrivacyNetworkGate::unexpectedOutboundCount() const {
    return static_cast<size_t>(std::count_if(ledger_.begin(), ledger_.end(), [](const auto& row) {
        return row.allowed && row.code != "network_request_authorized";
    }));
}

bool PrivacyNetworkGate::eraseLedger() {
    ledger_.clear();
    return ledger_.empty();
}

} // namespace urpg::privacy
