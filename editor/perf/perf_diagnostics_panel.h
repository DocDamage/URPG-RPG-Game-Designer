#pragma once

#include "editor/perf/perf_diagnostics_model.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

/**
 * @brief Lightweight panel that exposes a render snapshot for perf diagnostics.
 */
class PerfDiagnosticsPanel {
public:
    void bindModel(PerfDiagnosticsModel* model);
    void render();

    const nlohmann::json& lastRenderSnapshot() const { return m_snapshot; }

private:
    PerfDiagnosticsModel* m_model = nullptr;
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
