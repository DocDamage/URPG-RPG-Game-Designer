#pragma once

#include "editor/perf/perf_diagnostics_model.h"
#include "engine/core/playtest/playtest_performance_bridge.h"
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>

namespace urpg::editor {

/**
 * @brief Lightweight panel that exposes a render snapshot for perf diagnostics.
 */
class PerfDiagnosticsPanel {
public:
    void bindModel(PerfDiagnosticsModel* model);
    void bindLiveSession(const std::filesystem::path& session_directory);
    bool refreshLive(std::string* diagnostic = nullptr);
    bool requestLiveCapture(bool start, std::string capture_id = {}, std::string* diagnostic = nullptr);
    void render();

    const nlohmann::json& lastRenderSnapshot() const { return m_snapshot; }

private:
    PerfDiagnosticsModel* m_model = nullptr;
    std::unique_ptr<playtest::PlaytestPerformanceBridge> m_live_bridge;
    std::optional<playtest::LivePerformanceSnapshot> m_live_snapshot;
    uint64_t m_next_control_id = 1;
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
