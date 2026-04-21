#pragma once

#include "engine/core/perf/frame_budget.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::perf {

/**
 * @brief Tracks frame timing against a FrameBudget and reports budget violations.
 *
 * Thread-safety is NOT provided; intended for main-thread use only.
 */
class PerfProfiler {
public:
    explicit PerfProfiler(const FrameBudget& budget = FrameBudget{});

    void beginFrame();
    void endFrame();

    uint32_t getLastFrameUs() const;
    uint32_t getAverageFrameUs(size_t window = 60) const;

    bool wasBudgetViolated() const;
    uint32_t getViolationCount() const;

    nlohmann::json getBudgetSummary() const;

    void reset();

    void beginSection(const std::string& name);
    void endSection(const std::string& name);

    nlohmann::json getSectionSummary() const;

private:
    struct SectionTimings {
        std::vector<uint32_t> samples;
        size_t writeIndex = 0;
        size_t count = 0;
        uint32_t lastUs = 0;
    };

    static constexpr size_t kMaxSamples = 256;

    FrameBudget m_budget;
    std::chrono::time_point<std::chrono::steady_clock> m_frameStart;
    uint32_t m_lastFrameUs = 0;
    uint32_t m_violationCount = 0;
    bool m_budgetViolated = false;

    std::vector<uint32_t> m_frameTimes;
    size_t m_frameWriteIndex = 0;
    size_t m_frameCount = 0;

    std::map<std::string, SectionTimings> m_sections;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> m_activeSections;
};

} // namespace urpg::perf
