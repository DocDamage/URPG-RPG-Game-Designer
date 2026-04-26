#include "editor/perf/perf_diagnostics_panel.h"

namespace urpg::editor {

void PerfDiagnosticsPanel::bindModel(PerfDiagnosticsModel* model) {
    m_model = model;
}

void PerfDiagnosticsPanel::render() {
    if (!m_model) {
        m_snapshot = {
            {"panel", "perf_diagnostics"},
            {"status", "disabled"},
            {"disabled_reason", "No PerfDiagnosticsModel is bound."},
            {"owner", "editor/perf"},
            {"unlock_condition", "Bind PerfDiagnosticsModel before rendering performance diagnostics."},
        };
        return;
    }

    if (!m_model->hasProfiler()) {
        m_snapshot = {
            {"panel", "perf_diagnostics"},
            {"status", "disabled"},
            {"disabled_reason", "No PerfProfiler is attached to the diagnostics model."},
            {"owner", "editor/perf"},
            {"unlock_condition", "Attach PerfProfiler to PerfDiagnosticsModel before rendering performance diagnostics."},
        };
        return;
    }

    m_snapshot = nlohmann::json{
        {"panel", "perf_diagnostics"},
        {"status", "ready"},
        {"frame_summary", m_model->buildSnapshot()},
        {"sections", m_model->buildSectionSnapshot()}
    };
}

} // namespace urpg::editor
