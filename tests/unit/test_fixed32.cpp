#include "engine/core/math/fixed32.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Fixed32 multiplies deterministically", "[math]") {
    using urpg::Fixed32;

    const auto a = Fixed32::FromInt(3);
    const auto b = Fixed32::FromInt(2);
    const auto c = a * b;

    REQUIRE(c.raw == Fixed32::FromInt(6).raw);
}

TEST_CASE("Fixed32 handles negatives", "[math]") {
    using urpg::Fixed32;

    const auto a = Fixed32::FromInt(-3);
    const auto b = Fixed32::FromInt(2);

    REQUIRE((a * b).raw == Fixed32::FromInt(-6).raw);
}
