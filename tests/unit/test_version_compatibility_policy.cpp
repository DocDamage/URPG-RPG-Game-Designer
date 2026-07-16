#include "engine/core/export/version_compatibility_policy.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Version compatibility covers projects saves runtime extensions packages and updates",
          "[export][versioning][pcq753]") {
    using namespace urpg::exporting;
    std::vector<VersionCompatibilityRule> rules;
    for (const auto surface : {CompatibilitySurface::ProjectSchema, CompatibilitySurface::SaveData,
                               CompatibilitySurface::Runtime, CompatibilitySurface::PluginMod,
                               CompatibilitySurface::Package, CompatibilitySurface::UpdateChannel}) {
        rules.push_back({surface, "2.0.0", "1.0.0", "last_known_good_v1",
                         "This content was written by a newer version and cannot be safely downgraded.", true, true});
    }
    const auto result = VersionCompatibilityPolicy{}.evaluate(rules);
    REQUIRE(result.complete);
    REQUIRE(result.qualified);
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Version compatibility rejects invalid ranges and missing recovery proof", "[export][versioning][pcq753]") {
    using namespace urpg::exporting;
    const auto result = VersionCompatibilityPolicy{}.evaluate({
        {CompatibilitySurface::ProjectSchema, "1.0.0", "2.0.0", {}, {}, false, false}});
    REQUIRE_FALSE(result.complete);
    REQUIRE_FALSE(result.qualified);
    REQUIRE(result.diagnostics.size() == 10);
}
