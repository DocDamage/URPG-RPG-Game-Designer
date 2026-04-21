#include <catch2/catch_test_macros.hpp>
#include "engine/core/perf/perf_profiler.h"

#include <thread>

using namespace urpg;
using namespace urpg::perf;

TEST_CASE("PerfProfiler: beginFrame/endFrame computes plausible frame time", "[perf][profiler]") {
    PerfProfiler profiler;
    profiler.beginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.endFrame();

    REQUIRE(profiler.getLastFrameUs() >= 500);
}

TEST_CASE("PerfProfiler: average converges over multiple frames", "[perf][profiler]") {
    PerfProfiler profiler;
    for (int i = 0; i < 10; ++i) {
        profiler.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        profiler.endFrame();
    }

    REQUIRE(profiler.getAverageFrameUs(10) >= 500);
    REQUIRE(profiler.getAverageFrameUs(10) <= 50000);
}

TEST_CASE("PerfProfiler: budget violation is detected when simulated frame time exceeds budget", "[perf][profiler]") {
    FrameBudget budget;
    budget.total_us = 100;
    PerfProfiler profiler(budget);

    profiler.beginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    profiler.endFrame();

    REQUIRE(profiler.wasBudgetViolated());
    REQUIRE(profiler.getViolationCount() == 1);
}

TEST_CASE("PerfProfiler: reset clears history and violation count", "[perf][profiler]") {
    FrameBudget budget;
    budget.total_us = 100;
    PerfProfiler profiler(budget);

    profiler.beginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    profiler.endFrame();

    REQUIRE(profiler.getViolationCount() > 0);
    REQUIRE(profiler.getLastFrameUs() > 0);

    profiler.reset();

    REQUIRE(profiler.getLastFrameUs() == 0);
    REQUIRE(profiler.getViolationCount() == 0);
    REQUIRE_FALSE(profiler.wasBudgetViolated());
    REQUIRE(profiler.getAverageFrameUs() == 0);
}

TEST_CASE("PerfProfiler: named sections capture distinct timings", "[perf][profiler]") {
    PerfProfiler profiler;

    profiler.beginFrame();

    profiler.beginSection("section_a");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    profiler.endSection("section_a");

    profiler.beginSection("section_b");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.endSection("section_b");

    profiler.endFrame();

    auto summary = profiler.getSectionSummary();
    REQUIRE(summary.contains("section_a"));
    REQUIRE(summary.contains("section_b"));
    REQUIRE(summary["section_a"]["lastUs"].get<uint32_t>() >= 500);
    REQUIRE(summary["section_b"]["lastUs"].get<uint32_t>() >= 500);
}

TEST_CASE("PerfProfiler: getBudgetSummary returns valid JSON with expected fields", "[perf][profiler]") {
    PerfProfiler profiler;
    profiler.beginFrame();
    profiler.endFrame();

    auto summary = profiler.getBudgetSummary();
    REQUIRE(summary.contains("lastFrameUs"));
    REQUIRE(summary.contains("averageUs"));
    REQUIRE(summary.contains("violations"));
    REQUIRE(summary.contains("budgetTotalUs"));
    REQUIRE(summary.contains("withinBudget"));
    REQUIRE(summary["violations"].get<uint32_t>() == 0);
}

TEST_CASE("PerfProfiler: getSectionSummary returns per-section data", "[perf][profiler]") {
    PerfProfiler profiler;

    profiler.beginFrame();
    profiler.beginSection("render");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler.endSection("render");
    profiler.endFrame();

    auto summary = profiler.getSectionSummary();
    REQUIRE(summary.is_object());
    REQUIRE(summary.contains("render"));
    REQUIRE(summary["render"].contains("lastUs"));
    REQUIRE(summary["render"].contains("averageUs"));
}
