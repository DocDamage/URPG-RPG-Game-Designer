#include "engine/core/release/release_provenance.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Release provenance identifies exact channel build runtime package and support version",
          "[release][provenance][pcq756]") {
    const std::string hash(64, 'a');
    const auto result = urpg::release::ReleaseProvenanceBuilder{}.build({
        urpg::release::ReleaseChannel::Stable, "1.0.0", "win-x64-42", "abcdef12", "gcc-15.2.0",
        "windows-x64", hash, hash, false});
    REQUIRE(result.valid);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.metadata.at("channel") == "stable");
    REQUIRE(result.support_version == "1.0.0+win-x64-42.abcdef12");
}

TEST_CASE("Release provenance rejects dirty stable builds secrets paths and invalid hashes",
          "[release][provenance][pcq756]") {
    const auto result = urpg::release::ReleaseProvenanceBuilder{}.build({
        urpg::release::ReleaseChannel::Stable, "1.0.0", "C:/Users/dev/build", "api_key=secret",
        "gcc", "windows", "bad", "bad", true});
    REQUIRE_FALSE(result.valid);
    REQUIRE(result.diagnostics.size() == 5);
}
