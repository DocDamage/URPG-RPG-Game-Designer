#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::reliability {

enum class StressScenario : uint8_t {
    RepeatedOpenClose,
    RepeatedPlaytest,
    RepeatedPackage,
    LargeUndo,
    AssetChurn,
    DeviceChurn,
    SaveCycles,
    SuspendMinimize
};

struct StressSample {
    uint32_t iteration = 0;
    uint64_t memory_bytes = 0;
    uint32_t open_handles = 0;
    uint64_t undo_bytes = 0;
    uint32_t pending_jobs = 0;
    bool deadlock_detected = false;
    bool corruption_detected = false;
    bool responsive = true;
};

struct StressScenarioRun {
    std::string id;
    StressScenario scenario = StressScenario::RepeatedOpenClose;
    std::vector<StressSample> samples;
    std::string expected_final_hash;
    std::string actual_final_hash;
    bool completed = false;
};

struct StressLimits {
    uint32_t minimum_iterations = 100;
    uint64_t maximum_memory_growth_bytes = 64ULL * 1024 * 1024;
    uint32_t maximum_handle_growth = 16;
    uint64_t maximum_undo_bytes = 256ULL * 1024 * 1024;
    uint32_t maximum_pending_jobs = 32;
};

struct StressSuiteResult {
    bool complete = false;
    bool healthy = false;
    std::vector<std::string> diagnostics;
    nlohmann::json report;
};

class StressScenarioSuite {
public:
    explicit StressScenarioSuite(StressLimits limits = {});
    StressSuiteResult evaluate(std::vector<StressScenarioRun> runs) const;

private:
    StressLimits limits_;
};

const char* stressScenarioName(StressScenario scenario);

} // namespace urpg::reliability
