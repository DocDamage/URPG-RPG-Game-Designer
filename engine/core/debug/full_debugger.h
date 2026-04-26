#pragma once

#include <chrono>
#include <deque>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace urpg::debug {

/**
 * @brief A single data point for a variable in history.
 */
struct VariableSample {
    std::chrono::steady_clock::time_point timestamp;
    std::variant<int, float, std::string, bool> value;
};

/**
 * @brief Historical record of a variable's state over time.
 * Part of Phase 3 "Full Debugger" requirements.
 */
class VariableHistory {
  public:
    void push(const std::string& name, const std::variant<int, float, std::string, bool>& value) {
        auto& samples = m_history[name];
        samples.push_back({std::chrono::steady_clock::now(), value});

        // Keep history buffer size reasonable
        if (samples.size() > 1000)
            samples.pop_front();
    }

    const std::deque<VariableSample>& getHistory(const std::string& name) const {
        static const std::deque<VariableSample> empty;
        auto it = m_history.find(name);
        return (it != m_history.end()) ? it->second : empty;
    }

  private:
    std::map<std::string, std::deque<VariableSample>> m_history;
};

/**
 * @brief Performance profiler for tracking frame-time and subsystem overhead.
 */
struct ProfileMetric {
    std::string name;
    float durationMs;
    int callCount;
};

class EngineProfiler {
  public:
    void beginFrame() { m_frameStart = std::chrono::steady_clock::now(); }

    void endFrame() {
        auto end = std::chrono::steady_clock::now();
        m_lastFrameTimeMs = std::chrono::duration<float, std::milli>(end - m_frameStart).count();
    }

    void recordMetric(const std::string& subsystem, float ms) { m_metrics[subsystem] = {subsystem, ms, 1}; }

    float getLastFrameTime() const { return m_lastFrameTimeMs; }
    const std::map<std::string, ProfileMetric>& getMetrics() const { return m_metrics; }

  private:
    std::chrono::steady_clock::time_point m_frameStart;
    float m_lastFrameTimeMs = 0.0f;
    std::map<std::string, ProfileMetric> m_metrics;
};

} // namespace urpg::debug
