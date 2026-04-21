#include "editor/perf/perf_diagnostics_panel.h"

namespace urpg::editor {

void PerfDiagnosticsPanel::bindModel(PerfDiagnosticsModel* model) {
    m_model = model;
}

void PerfDiagnosticsPanel::render() {
    if (!m_model || !m_model->hasProfiler()) {
        m_snapshot = nlohmann::json::object();
        return;
    }

    m_snapshot = nlohmann::json{
        {"frame_summary", m_model->buildSnapshot()},
        {"sections", m_model->buildSectionSnapshot()}
    };
}

} // namespace urpg::editor
