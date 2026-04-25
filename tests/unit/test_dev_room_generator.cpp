#include "engine/core/project/dev_room_generator.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("dev room scripted route visits all onboarding stations", "[project][dev_room][ffs08]") {
    const urpg::project::DevRoomGenerator generator;

    const auto result = generator.generate("demo_project");

    REQUIRE(result.room["stations"].size() == 9);
    REQUIRE(result.room["scripted_route"].size() == 9);
    REQUIRE(generator.validateScriptedRoute(result.room).empty());
    REQUIRE(result.audit_report["status"] == "passed");
}

TEST_CASE("dev room route validation reports missing station while preserving audit shape", "[project][dev_room][ffs08]") {
    const urpg::project::DevRoomGenerator generator;
    auto result = generator.generate("demo_project");
    result.room["scripted_route"].erase(0);

    const auto errors = generator.validateScriptedRoute(result.room);

    REQUIRE(std::find(errors.begin(), errors.end(), "route_missing_station:message") != errors.end());
    REQUIRE(result.audit_report.contains("failures"));
}
