#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::privacy {

enum class NetworkPurpose : uint8_t { Telemetry, SupportUpload, AiProvider, UpdateCheck, Multiplayer };

struct PrivacyPolicy {
    bool offline_first = true;
    bool automatic_upload = false;
    bool require_preview = true;
    bool require_explicit_consent = true;
    bool telemetry_opt_in = true;
    bool retention_configured = true;
    bool deletion_supported = true;
    uint32_t maximum_retention_days = 30;
};

struct NetworkRequest {
    std::string id;
    NetworkPurpose purpose = NetworkPurpose::Telemetry;
    std::string endpoint_profile_id;
    bool user_initiated = false;
    bool preview_approved = false;
    bool explicit_consent = false;
};

struct NetworkDecision {
    std::string request_id;
    NetworkPurpose purpose = NetworkPurpose::Telemetry;
    bool allowed = false;
    std::string code;
    std::string message;
};

class PrivacyNetworkGate {
public:
    explicit PrivacyNetworkGate(PrivacyPolicy policy = {});

    NetworkDecision authorize(const NetworkRequest& request);
    const std::vector<NetworkDecision>& ledger() const { return ledger_; }
    size_t allowedCount() const;
    size_t unexpectedOutboundCount() const;
    bool eraseLedger();
    const PrivacyPolicy& policy() const { return policy_; }

private:
    PrivacyPolicy policy_;
    std::vector<NetworkDecision> ledger_;
};

const char* networkPurposeName(NetworkPurpose purpose);

} // namespace urpg::privacy
