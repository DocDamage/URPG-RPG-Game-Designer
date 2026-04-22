#include <catch2/catch_test_macros.hpp>
#include "engine/core/testing/snapshot_validator.h"

using namespace urpg::testing;

TEST_CASE("SnapshotValidator detects pixel differences", "[testing][snapshot]") {
    SECTION("Identical snapshots match") {
        SceneSnapshot s1 = {2, 2, {{255,0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,255,255}}};
        SceneSnapshot s2 = s1;

        auto result = SnapshotValidator::compare(s1, s2);
        REQUIRE(result.matches);
        REQUIRE(result.errorPercentage == 0.0f);
    }

    SECTION("Different snapshots fail") {
        SceneSnapshot s1 = {2, 2, {{255,0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,255,255}}};
        SceneSnapshot s2 = {2, 2, {{255,0,0,255}, {0,252,0,255}, {0,0,255,255}, {255,255,255,255}}}; // One pixel change

        auto result = SnapshotValidator::compare(s1, s2, 0.0f); // Zero threshold
        REQUIRE_FALSE(result.matches);
        REQUIRE(result.errorPercentage == 25.0f); // 1 out of 4 pixels
    }

    SECTION("Mismatched dimensions fail immediately") {
        SceneSnapshot s1 = {2, 2, {{255,0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,255,255}}};
        SceneSnapshot s2 = {1, 1, {{255,0,0,255}}};

        auto result = SnapshotValidator::compare(s1, s2);
        REQUIRE_FALSE(result.matches);
        REQUIRE(result.errorPercentage == 100.0f);
    }

    SECTION("Respects threshold for minor differences") {
        SceneSnapshot s1 = {10, 10}; // 100 pixels
        s1.pixels.assign(100, {255,255,255,255});
        
        SceneSnapshot s2 = s1;
        s2.pixels[0] = {0,0,0,255}; // 1% difference
        
        auto match = SnapshotValidator::compare(s1, s2, 1.0f); // 1% threshold
        REQUIRE(match.matches);
        
        auto fail = SnapshotValidator::compare(s1, s2, 0.5f); // 0.5% threshold
        REQUIRE_FALSE(fail.matches);
    }
}
