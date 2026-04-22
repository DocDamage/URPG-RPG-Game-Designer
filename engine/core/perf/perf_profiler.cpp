#include "engine/core/perf/perf_profiler.h"

#include <algorithm>

namespace urpg::perf {

PerfProfiler::PerfProfiler(const FrameBudget& budget)
    : m_budget(budget)
    , m_frameTimes(kMaxSamples, 0) {}

void PerfProfiler::beginFrame() {
    m_frameStart = std::chrono::steady_clock::now();
    m_budgetViolated = false;
}

void PerfProfiler::endFrame() {
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - m_frameStart).count();
    m_lastFrameUs = static_cast<uint32_t>(elapsed);

    m_frameTimes[m_frameWriteIndex] = m_lastFrameUs;
    m_frameWriteIndex = (m_frameWriteIndex + 1) % kMaxSamples;
    if (m_frameCount < kMaxSamples) {
        ++m_frameCount;
    }

    if (m_lastFrameUs > m_budget.total_us) {
        m_budgetViolated = true;
        ++m_violationCount;
    }
}

uint32_t PerfProfiler::getLastFrameUs() const {
    return m_lastFrameUs;
}

uint32_t PerfProfiler::getAverageFrameUs(size_t window) const {
    if (m_frameCount == 0) {
        return 0;
    }

    const size_t samples = std::min(window, m_frameCount);
    uint64_t sum = 0;
    for (size_t i = 0; i < samples; ++i) {
        const size_t idx = (m_frameWriteIndex + kMaxSamples - 1 - i) % kMaxSamples;
        sum += m_frameTimes[idx];
    }
    return static_cast<uint32_t>(sum / samples);
}

bool PerfProfiler::wasBudgetViolated() const {
    return m_budgetViolated;
}

uint32_t PerfProfiler::getViolationCount() const {
    return m_violationCount;
}

nlohmann::json PerfProfiler::getBudgetSummary() const {
    return nlohmann::json{
        {"lastFrameUs", m_lastFrameUs},
        {"averageUs", getAverageFrameUs()},
        {"violations", m_violationCount},
        {"budgetTotalUs", m_budget.total_us},
        {"withinBudget", !m_budgetViolated}
    };
}

void PerfProfiler::reset() {
    m_lastFrameUs = 0;
    m_violationCount = 0;
    m_budgetViolated = false;
    m_frameWriteIndex = 0;
    m_frameCount = 0;
    std::fill(m_frameTimes.begin(), m_frameTimes.end(), 0);
    m_sections.clear();
    m_activeSections.clear();
}

void PerfProfiler::beginSection(const std::string& name) {
    m_activeSections[name] = std::chrono::steady_clock::now();
}

void PerfProfiler::endSection(const std::string& name) {
    const auto it = m_activeSections.find(name);
    if (it == m_activeSections.end()) {
        return;
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second).count();
    m_activeSections.erase(it);

    auto& section = m_sections[name];
    if (section.samples.empty()) {
        section.samples.resize(kMaxSamples, 0);
    }
    section.samples[section.writeIndex] = static_cast<uint32_t>(elapsed);
    section.writeIndex = (section.writeIndex + 1) % kMaxSamples;
    section.lastUs = static_cast<uint32_t>(elapsed);
    if (section.count < kMaxSamples) {
        ++section.count;
    }
}

nlohmann::json PerfProfiler::getSectionSummary() const {
    nlohmann::json result = nlohmann::json::object();
    for (const auto& [name, section] : m_sections) {
        const size_t samples = std::min(section.count, section.samples.size());
        uint64_t sum = 0;
        for (size_t i = 0; i < samples; ++i) {
            const size_t idx = (section.writeIndex + kMaxSamples - 1 - i) % kMaxSamples;
            sum += section.samples[idx];
        }
        const uint32_t avg = samples > 0 ? static_cast<uint32_t>(sum / samples) : 0;
        result[name] = nlohmann::json{
            {"lastUs", section.lastUs},
            {"averageUs", avg}
        };
    }
    return result;
}

} // namespace urpg::perf
