#include "engine/core/privacy/privacy_network_policy.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Privacy defaults are offline first and block every automatic upload", "[privacy][network][pcq706]") {
    urpg::privacy::PrivacyNetworkGate gate;
    REQUIRE(gate.policy().offline_first);
    REQUIRE_FALSE(gate.policy().automatic_upload);
    REQUIRE(gate.policy().require_preview);
    REQUIRE(gate.policy().require_explicit_consent);
    REQUIRE(gate.policy().telemetry_opt_in);
    REQUIRE(gate.policy().retention_configured);
    REQUIRE(gate.policy().deletion_supported);

    for (const auto purpose : {urpg::privacy::NetworkPurpose::Telemetry,
                               urpg::privacy::NetworkPurpose::SupportUpload,
                               urpg::privacy::NetworkPurpose::AiProvider,
                               urpg::privacy::NetworkPurpose::UpdateCheck,
                               urpg::privacy::NetworkPurpose::Multiplayer}) {
        const auto decision = gate.authorize({std::string("automatic-") + urpg::privacy::networkPurposeName(purpose),
                                              purpose, "reviewed-profile", false, true, true});
        REQUIRE_FALSE(decision.allowed);
        REQUIRE(decision.code == "network_automatic_request_blocked");
    }
    REQUIRE(gate.allowedCount() == 0);
    REQUIRE(gate.unexpectedOutboundCount() == 0);
}

TEST_CASE("Privacy gate allows only previewed consented user requests and supports deletion",
          "[privacy][network][pcq706]") {
    urpg::privacy::PrivacyNetworkGate gate;
    REQUIRE_FALSE(gate.authorize({"support", urpg::privacy::NetworkPurpose::SupportUpload,
                                  "support-v1", true, false, true}).allowed);
    REQUIRE_FALSE(gate.authorize({"ai", urpg::privacy::NetworkPurpose::AiProvider,
                                  "provider-v1", true, true, false}).allowed);
    const auto approved = gate.authorize({"support-approved", urpg::privacy::NetworkPurpose::SupportUpload,
                                          "support-v1", true, true, true});
    REQUIRE(approved.allowed);
    REQUIRE(gate.allowedCount() == 1);
    REQUIRE(gate.unexpectedOutboundCount() == 0);
    REQUIRE(gate.eraseLedger());
    REQUIRE(gate.ledger().empty());
}
