#include "engine/core/security/release_sbom.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

std::vector<urpg::security::SbomComponent> completeComponents() {
    using urpg::security::SbomComponentClass;
    const std::string hash(64, 'a');
    return {{"engine", "URPG Engine", "0.1.0", SbomComponentClass::Binary, hash, "MIT", "https://example.invalid/urpg", "", true, true, false},
            {"pack-tool", "Package Tool", "1.0", SbomComponentClass::Tool, hash, "Apache-2.0", "https://example.invalid/tool", "Tool notice", false, true, false},
            {"hero-art", "Hero Art", "asset-v1", SbomComponentClass::PackagedAsset, hash, "CC0-1.0", "https://example.invalid/art", "", true, true, false},
            {"ai-provider", "Optional AI Provider", "api-v1", SbomComponentClass::OptionalProvider, hash, "LicenseRef-Service", "https://example.invalid/provider", "Not redistributed.", false, false, true},
            {"research", "Research Motion Output", "pilot-v1", SbomComponentClass::ResearchOutput, hash, "LicenseRef-Review", "https://example.invalid/research", "Excluded pending adoption.", false, false, true}};
}

} // namespace

TEST_CASE("Release SBOM deterministically covers binaries tools assets providers and research", "[security][sbom][pcq707]") {
    const auto components = completeComponents();
    auto reversed = components;
    std::reverse(reversed.begin(), reversed.end());
    const auto first = urpg::security::ReleaseSbomBuilder{}.build("0.1.0-dev", components);
    const auto second = urpg::security::ReleaseSbomBuilder{}.build("0.1.0-dev", reversed);
    REQUIRE(first.complete);
    REQUIRE(first.release_allowed);
    REQUIRE(first.diagnostics.empty());
    REQUIRE(first.sbom == second.sbom);
    REQUIRE(first.notices == second.notices);
    REQUIRE(first.sbom.at("packages").size() == 5);
    REQUIRE(first.notices.find("Optional AI Provider") != std::string::npos);
}

TEST_CASE("Release SBOM blocks missing provenance hashes licenses and approvals", "[security][sbom][pcq707]") {
    auto components = completeComponents();
    components[0].sha256 = "bad";
    components[1].source.clear();
    components[2].license_id.clear();
    components[2].redistribution_approved = false;
    const auto result = urpg::security::ReleaseSbomBuilder{}.build("0.1.0-dev", components);
    REQUIRE(result.complete);
    REQUIRE_FALSE(result.release_allowed);
    REQUIRE(result.diagnostics.size() == 4);
}
