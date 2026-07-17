#include "engine/core/perf/product_benchmark_suite.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::vector<urpg::perf::BenchmarkMeasurement> completeMeasurements(
    const std::vector<urpg::perf::BenchmarkProjectFixture>& fixtures,
    const std::vector<urpg::perf::BenchmarkHardwareProfile>& hardware) {
    using urpg::perf::BenchmarkMetric;
    const BenchmarkMetric metrics[] = {BenchmarkMetric::Startup, BenchmarkMetric::ProjectOpen, BenchmarkMetric::Save,
                                       BenchmarkMetric::Search, BenchmarkMetric::MapEdit, BenchmarkMetric::PlaytestLaunch,
                                       BenchmarkMetric::FramePacing, BenchmarkMetric::Package, BenchmarkMetric::Memory};
    std::vector<urpg::perf::BenchmarkMeasurement> rows;
    for (const auto& fixture : fixtures) {
        for (const auto& profile : hardware) {
            for (const auto metric : metrics) rows.push_back({fixture.id, profile.id, metric, 900, 1000, 5});
        }
    }
    return rows;
}

} // namespace

TEST_CASE("Product benchmark suite versions complete tiny medium large hardware matrices", "[perf][benchmark][pcq700]") {
    const auto fixtures = urpg::perf::ProductBenchmarkSuite::representativeFixtures();
    const auto hardware = urpg::perf::ProductBenchmarkSuite::targetHardwareClasses();
    const auto result = urpg::perf::ProductBenchmarkSuite{}.evaluate(
        "pcq700.v1", fixtures, hardware, completeMeasurements(fixtures, hardware));

    REQUIRE(result.complete);
    REQUIRE(result.within_thresholds);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.report.at("measurements").size() == 81);
    REQUIRE(result.report.at("baseline_version") == "pcq700.v1");
    REQUIRE(result.report.dump().find("frame_pacing") != std::string::npos);
    REQUIRE(result.report.dump().find("memory") != std::string::npos);
}

TEST_CASE("Product benchmark suite makes regressions and missing coverage visible", "[perf][benchmark][pcq700]") {
    const auto fixtures = urpg::perf::ProductBenchmarkSuite::representativeFixtures();
    const auto hardware = urpg::perf::ProductBenchmarkSuite::targetHardwareClasses();
    auto rows = completeMeasurements(fixtures, hardware);
    rows.front().measured_value = 1001;
    rows.pop_back();
    const auto result = urpg::perf::ProductBenchmarkSuite{}.evaluate("pcq700.v1", fixtures, hardware, rows);

    REQUIRE_FALSE(result.complete);
    REQUIRE_FALSE(result.within_thresholds);
    REQUIRE_FALSE(result.diagnostics.empty());
    REQUIRE(result.report.dump().find("benchmark_measurement_missing") != std::string::npos);
    bool sawRegression = false;
    for (const auto& row : result.report.at("measurements")) {
        sawRegression = sawRegression || !row.at("passes").get<bool>();
    }
    REQUIRE(sawRegression);
}

TEST_CASE("Product benchmark plan is file-backed and expands every governed threshold",
          "[perf][benchmark][pcq700]") {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "benchmarks" /
                        "product_benchmark_plan_v1.json");
    REQUIRE(input.good());
    const auto plan = urpg::perf::ProductBenchmarkSuite::parsePlan(nlohmann::json::parse(input));
    REQUIRE(plan.valid);
    REQUIRE(plan.diagnostics.empty());
    REQUIRE(plan.baseline_version == "pcq700.v1");
    REQUIRE(plan.threshold_status == "provisional_pending_target_hardware_capture_and_owner_approval");
    REQUIRE(plan.fixtures.size() == 3);
    REQUIRE(plan.hardware.size() == 3);
    REQUIRE(plan.thresholds.size() == 81);

    auto rows = completeMeasurements(plan.fixtures, plan.hardware);
    for (auto& row : rows) {
        row.measured_value = 1;
        row.regression_threshold = 0;
    }
    const auto result = urpg::perf::ProductBenchmarkSuite{}.evaluate(plan, std::move(rows));
    REQUIRE(result.complete);
    REQUIRE(result.within_thresholds);
    REQUIRE(result.report.at("threshold_status") == plan.threshold_status);
    REQUIRE(result.report.at("measurements").size() == 81);
}
