#include "editor/perf/perf_diagnostics_model.h"

namespace urpg::editor {

void PerfDiagnosticsModel::attachProfiler(urpg::perf::PerfProfiler* profiler) {
    m_profiler = profiler;
}

void PerfDiagnosticsModel::detachProfiler() {
    m_profiler = nullptr;
}

nlohmann::json PerfDiagnosticsModel::buildSnapshot() const {
    if (!m_profiler) {
        return nlohmann::json::object();
    }
    return m_profiler->getBudgetSummary();
}

nlohmann::json PerfDiagnosticsModel::buildSectionSnapshot() const {
    if (!m_profiler) {
        return nlohmann::json::object();
    }
    return m_profiler->getSectionSummary();
}

bool PerfDiagnosticsModel::hasProfiler() const {
    return m_profiler != nullptr;
}

} // namespace urpg::editor
