#include "engine/core/security/release_security_audit.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Release security audit covers every trust boundary with adversarial evidence",
          "[security][release_audit][pcq705]") {
    using namespace urpg::security;
    std::vector<SecurityControlEvidence> evidence;
    for (const auto surface : {SecuritySurface::ExternalProcess, SecuritySurface::ArchiveExtraction,
                               SecuritySurface::PathContainment, SecuritySurface::PluginModTrust,
                               SecuritySurface::Secrets, SecuritySurface::NetworkDefaults,
                               SecuritySurface::Logs, SecuritySurface::SupportBundles}) {
        evidence.push_back({surface, "owner:" + std::string(securitySurfaceName(surface)),
                            "Validate, constrain, redact, and fail closed.",
                            "fixture:" + std::string(securitySurfaceName(surface)), true, true, true});
    }
    const auto result = ReleaseSecurityAudit{}.evaluate(evidence);
    REQUIRE(result.complete);
    REQUIRE(result.safe);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.threat_model.at("controls").size() == 8);

    REQUIRE_FALSE(ReleaseSecurityAudit::isUnsafeRelativePath("content/maps/field.json"));
    REQUIRE(ReleaseSecurityAudit::isUnsafeRelativePath("../secrets.env"));
    REQUIRE(ReleaseSecurityAudit::isUnsafeRelativePath("C:/Users/example/token.txt"));
    REQUIRE(ReleaseSecurityAudit::containsLikelySecret("Authorization: Bearer abc"));
    REQUIRE_FALSE(ReleaseSecurityAudit::containsLikelySecret("ordinary diagnostic text"));
}

TEST_CASE("Release security audit rejects permissive or incomplete threat models",
          "[security][release_audit][pcq705]") {
    using namespace urpg::security;
    const auto result = ReleaseSecurityAudit{}.evaluate({
        {SecuritySurface::ExternalProcess, "process", "shell allowlist", "metacharacters", false, false, false}});
    REQUIRE_FALSE(result.complete);
    REQUIRE_FALSE(result.safe);
    REQUIRE(result.diagnostics.size() == 10);
}
