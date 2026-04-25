#include "engine/core/rest/rest_point.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Rest point applies recovery costs and reports affordability", "[rest][balance][ffs12]") {
    urpg::rest::RestPoint inn{"inn.town", 30, 100, 40};

    const auto denied = inn.preview(20, 25, 0);
    const auto accepted = inn.preview(50, 25, 0);

    REQUIRE_FALSE(denied.affordable);
    REQUIRE(accepted.affordable);
    REQUIRE(accepted.gold_after == 20);
    REQUIRE(accepted.hp_after == 100);
    REQUIRE(accepted.mp_after == 40);
}
