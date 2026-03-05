#include "engine/core/semver.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("SemVer parses valid versions", "[semver]") {
    urpg::SemVer version;
    REQUIRE(urpg::SemVer::TryParse("1.2.3", version));
    REQUIRE(version.major == 1);
    REQUIRE(version.minor == 2);
    REQUIRE(version.patch == 3);
    REQUIRE(version.ToString() == "1.2.3");
}

TEST_CASE("SemVer rejects invalid versions", "[semver]") {
    urpg::SemVer version;
    REQUIRE_FALSE(urpg::SemVer::TryParse("1.2", version));
    REQUIRE_FALSE(urpg::SemVer::TryParse("a.b.c", version));
}
