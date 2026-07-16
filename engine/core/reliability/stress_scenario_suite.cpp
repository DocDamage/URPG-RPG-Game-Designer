#include "engine/core/reliability/stress_scenario_suite.h"

#include <algorithm>
#include <array>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::reliability {
namespace {

constexpr std::array<StressScenario, 8> kRequiredScenarios = {
    StressScenario::RepeatedOpenClose, StressScenario::RepeatedPlaytest, StressScenario::RepeatedPackage,
    StressScenario::LargeUndo, StressScenario::AssetChurn, StressScenario::DeviceChurn,
    StressScenario::SaveCycles, StressScenario::SuspendMinimize};

} // namespace

const char* stressScenarioName(const StressScenario scenario) {
    switch (scenario) {
    case StressScenario::RepeatedOpenClose: return "repeated_open_close";
    case StressScenario::RepeatedPlaytest: return "repeated_playtest";
    case StressScenario::RepeatedPackage: return "repeated_package";
    case StressScenario::LargeUndo: return "large_undo";
    case StressScenario::AssetChurn: return "asset_churn";
    case StressScenario::DeviceChurn: return "device_churn";
    case StressScenario::SaveCycles: return "save_cycles";
    case StressScenario::SuspendMinimize: return "suspend_minimize";
    }
    return "unknown";
}

StressScenarioSuite::StressScenarioSuite(StressLimits limits) : limits_(limits) {
    limits_.minimum_iterations = std::max<uint32_t>(1, limits_.minimum_iterations);
    limits_.maximum_memory_growth_bytes = std::max<uint64_t>(1, limits_.maximum_memory_growth_bytes);
    limits_.maximum_handle_growth = std::max<uint32_t>(1, limits_.maximum_handle_growth);
    limits_.maximum_undo_bytes = std::max<uint64_t>(1, limits_.maximum_undo_bytes);
    limits_.maximum_pending_jobs = std::max<uint32_t>(1, limits_.maximum_pending_jobs);
}

StressSuiteResult StressScenarioSuite::evaluate(std::vector<StressScenarioRun> runs) const {
    StressSuiteResult result;
    std::stable_sort(runs.begin(), runs.end(), [](const auto& left, const auto& right) {
        return std::tie(left.scenario, left.id) < std::tie(right.scenario, right.id);
    });
    std::set<std::string> ids;
    std::set<StressScenario> covered;
    nlohmann::json reportRuns = nlohmann::json::array();
    for (const auto& run : runs) {
        const auto prefix = std::string(stressScenarioName(run.scenario));
        if (run.id.empty() || !ids.insert(run.id).second || !covered.insert(run.scenario).second) {
            result.diagnostics.push_back("stress_run_invalid_or_duplicate:" + run.id);
        }
        if (!run.completed) result.diagnostics.push_back("stress_run_incomplete:" + prefix);
        if (run.samples.size() < limits_.minimum_iterations) result.diagnostics.push_back("stress_iterations_insufficient:" + prefix);
        if (run.expected_final_hash.empty() || run.actual_final_hash != run.expected_final_hash) {
            result.diagnostics.push_back("stress_state_hash_mismatch:" + prefix);
        }

        uint64_t minimumMemory = UINT64_MAX;
        uint64_t maximumMemory = 0;
        uint32_t minimumHandles = UINT32_MAX;
        uint32_t maximumHandles = 0;
        uint64_t maximumUndo = 0;
        uint32_t maximumJobs = 0;
        uint32_t previousIteration = 0;
        bool first = true;
        for (const auto& sample : run.samples) {
            if ((!first && sample.iteration <= previousIteration) || sample.memory_bytes == 0) {
                result.diagnostics.push_back("stress_sample_order_invalid:" + prefix);
                break;
            }
            first = false;
            previousIteration = sample.iteration;
            minimumMemory = std::min(minimumMemory, sample.memory_bytes);
            maximumMemory = std::max(maximumMemory, sample.memory_bytes);
            minimumHandles = std::min(minimumHandles, sample.open_handles);
            maximumHandles = std::max(maximumHandles, sample.open_handles);
            maximumUndo = std::max(maximumUndo, sample.undo_bytes);
            maximumJobs = std::max(maximumJobs, sample.pending_jobs);
            if (sample.deadlock_detected) result.diagnostics.push_back("stress_deadlock:" + prefix);
            if (sample.corruption_detected) result.diagnostics.push_back("stress_corruption:" + prefix);
            if (!sample.responsive) result.diagnostics.push_back("stress_unresponsive:" + prefix);
        }
        if (!run.samples.empty()) {
            if (maximumMemory - minimumMemory > limits_.maximum_memory_growth_bytes) {
                result.diagnostics.push_back("stress_memory_growth_unbounded:" + prefix);
            }
            if (maximumHandles - minimumHandles > limits_.maximum_handle_growth) {
                result.diagnostics.push_back("stress_handle_growth_unbounded:" + prefix);
            }
            if (maximumUndo > limits_.maximum_undo_bytes) result.diagnostics.push_back("stress_undo_unbounded:" + prefix);
            if (maximumJobs > limits_.maximum_pending_jobs) result.diagnostics.push_back("stress_jobs_unbounded:" + prefix);
            if (run.samples.back().pending_jobs != 0) result.diagnostics.push_back("stress_job_unrecovered:" + prefix);
        }
        reportRuns.push_back({{"id", run.id}, {"scenario", prefix}, {"iterations", run.samples.size()},
                              {"completed", run.completed}, {"minimum_memory_bytes", minimumMemory},
                              {"maximum_memory_bytes", maximumMemory}, {"maximum_open_handles", maximumHandles},
                              {"maximum_undo_bytes", maximumUndo}, {"maximum_pending_jobs", maximumJobs},
                              {"expected_final_hash", run.expected_final_hash},
                              {"actual_final_hash", run.actual_final_hash}});
    }
    for (const auto scenario : kRequiredScenarios) {
        if (!covered.contains(scenario)) result.diagnostics.push_back(std::string("stress_scenario_missing:") + stressScenarioName(scenario));
    }
    result.complete = covered.size() == kRequiredScenarios.size();
    result.healthy = result.complete && result.diagnostics.empty();
    result.report = {{"schema", "urpg.stress_suite.v1"}, {"complete", result.complete},
                     {"healthy", result.healthy}, {"runs", std::move(reportRuns)},
                     {"diagnostics", result.diagnostics}};
    return result;
}

} // namespace urpg::reliability
