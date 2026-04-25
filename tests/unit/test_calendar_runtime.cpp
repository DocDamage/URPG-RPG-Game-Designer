#include "engine/core/time/calendar_runtime.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Calendar event fires exactly once per scheduled window", "[calendar][simulation][ffs13]") {
    urpg::time::CalendarRuntime calendar;
    calendar.addEvent({"festival", 2, 20, 30, "festival_started"});

    REQUIRE(calendar.advanceTo(2, 25).triggeredFlags == std::vector<std::string>{"festival_started"});
    REQUIRE(calendar.advanceTo(2, 26).triggeredFlags.empty());
    REQUIRE(calendar.advanceTo(3, 25).triggeredFlags == std::vector<std::string>{"festival_started"});
}

TEST_CASE("Calendar supports events that cross day boundaries", "[calendar][simulation][ffs13]") {
    urpg::time::CalendarRuntime calendar;
    calendar.addEvent({"night_watch", 4, 22, 2, "watch"});

    REQUIRE(calendar.advanceTo(4, 23).triggeredFlags == std::vector<std::string>{"watch"});
    REQUIRE(calendar.advanceTo(5, 1).triggeredFlags.empty());
    REQUIRE(calendar.validate().empty());
}
