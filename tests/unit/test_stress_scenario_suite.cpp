#include "engine/core/reliability/stress_scenario_suite.h"

#include <catch2/catch_test_macros.hpp>

namespace {

std::vector<urpg::reliability::StressScenarioRun> healthyRuns() {
    using urpg::reliability::StressScenario;
    std::vector<urpg::reliability::StressScenarioRun> runs;
    for (const auto scenario : {StressScenario::RepeatedOpenClose, StressScenario::RepeatedPlaytest,
                                StressScenario::RepeatedPackage, StressScenario::LargeUndo,
                                StressScenario::AssetChurn, StressScenario::DeviceChurn,
                                StressScenario::SaveCycles, StressScenario::SuspendMinimize}) {
        urpg::reliability::StressScenarioRun run;
        run.id = std::string(urpg::reliability::stressScenarioName(scenario)) + "-100";
        run.scenario = scenario;
        run.completed = true;
        run.expected_final_hash = "stable-project-hash";
        run.actual_final_hash = run.expected_final_hash;
        for (uint32_t iteration = 1; iteration <= 100; ++iteration) {
            run.samples.push_back({iteration, 256ULL * 1024 * 1024 + iteration * 1024, 24 + iteration % 2,
                                   scenario == StressScenario::LargeUndo ? iteration * 1024ULL : 0,
                                   iteration == 100 ? 0U : iteration % 3, false, false, true});
        }
        runs.push_back(std::move(run));
    }
    return runs;
}

} // namespace

TEST_CASE("Stress suite covers long-session lifecycle churn without unbounded state", "[reliability][stress][pcq702]") {
    const auto result = urpg::reliability::StressScenarioSuite{}.evaluate(healthyRuns());
    REQUIRE(result.complete);
    REQUIRE(result.healthy);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.report.at("runs").size() == 8);
    REQUIRE(result.report.dump().find("suspend_minimize") != std::string::npos);
}

TEST_CASE("Stress suite reports growth corruption deadlocks and unrecovered jobs", "[reliability][stress][pcq702]") {
    auto runs = healthyRuns();
    runs[0].samples.back().memory_bytes += 128ULL * 1024 * 1024;
    runs[1].samples[50].deadlock_detected = true;
    runs[2].samples[50].corruption_detected = true;
    runs[3].samples.back().pending_jobs = 2;
    const auto result = urpg::reliability::StressScenarioSuite{}.evaluate(std::move(runs));
    REQUIRE(result.complete);
    REQUIRE_FALSE(result.healthy);
    REQUIRE(result.diagnostics.size() == 4);
}
