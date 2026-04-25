#include "engine/core/narrative/ending_route_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ending route manager chooses highest priority eligible ending deterministically", "[narrative][ending][ffs10]") {
    urpg::narrative::EndingRouteManager manager;
    manager.addRoute({"neutral", 1, {{"trust", 0}}});
    manager.addRoute({"golden", 10, {{"trust", 50}}});
    manager.addRoute({"golden_alt", 10, {{"trust", 50}}});

    const auto ending = manager.evaluate({{"trust", 60}});

    REQUIRE(ending.has_value());
    REQUIRE(ending->id == "golden");
}
