#pragma once

#include "engine/core/perf/perf_profiler.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

/**
 * @brief Wraps PerfProfiler for diagnostics workspace consumption.
 */
class PerfDiagnosticsModel {
public:
    void attachProfiler(urpg::perf::PerfProfiler* profiler);
    void detachProfiler();

    nlohmann::json buildSnapshot() const;
    nlohmann::json buildSectionSnapshot() const;
    bool hasProfiler() const;

private:
    urpg::perf::PerfProfiler* m_profiler = nullptr;
};

} // namespace urpg::editor
