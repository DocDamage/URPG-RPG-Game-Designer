#include "engine/core/perf/frame_budget.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("FrameBudget totals sum correctly", "[perf]") {
    urpg::FrameBudget budget;
    REQUIRE(budget.native_reserve + budget.plugin_pool + budget.headroom == budget.total_us);
}
