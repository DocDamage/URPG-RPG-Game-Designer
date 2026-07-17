#include "editor/perf/perf_diagnostics_panel.h"

#include <algorithm>

namespace urpg::editor {

void PerfDiagnosticsPanel::bindModel(PerfDiagnosticsModel* model) {
    m_model = model;
}

void PerfDiagnosticsPanel::bindLiveSession(const std::filesystem::path& session_directory) {
    m_live_bridge = std::make_unique<playtest::PlaytestPerformanceBridge>(session_directory);
    m_live_snapshot.reset();
    m_next_control_id = 1;
}

bool PerfDiagnosticsPanel::refreshLive(std::string* diagnostic) {
    if (!m_live_bridge) return false;
    const auto next = m_live_bridge->readAfter(m_live_snapshot ? m_live_snapshot->revision : 0, diagnostic);
    if (next) {
        m_live_snapshot = *next;
        m_next_control_id = std::max(m_next_control_id, next->last_control_id + 1);
    }
    return next.has_value() || diagnostic == nullptr || diagnostic->empty();
}

bool PerfDiagnosticsPanel::requestLiveCapture(const bool start, std::string capture_id,
                                              std::string* diagnostic) {
    if (!m_live_bridge || !m_live_snapshot) return false;
    if (start && capture_id.empty()) capture_id = m_live_snapshot->session_id + "-manual";
    const playtest::PerformanceControl control{
        m_next_control_id, m_live_snapshot->revision,
        start ? playtest::PerformanceControlAction::Start : playtest::PerformanceControlAction::Stop,
        std::move(capture_id)};
    if (!m_live_bridge->appendControl(control, diagnostic)) return false;
    ++m_next_control_id;
    return true;
}

void PerfDiagnosticsPanel::render() {
    (void)refreshLive();
    if (m_live_snapshot) {
        const auto& overlay = m_live_snapshot->overlay;
        nlohmann::json subsystems = nlohmann::json::object();
        for (const auto& [kind, time] : overlay.latest_subsystem_us) {
            subsystems[perf::performanceSubsystemName(kind)] = time;
        }
        nlohmann::json spike;
        if (overlay.latest_spike) {
            spike = {{"frame_index", overlay.latest_spike->frame_index},
                     {"timestamp_us", overlay.latest_spike->timestamp_us},
                     {"frame_time_us", overlay.latest_spike->frame_time_us},
                     {"dominant_subsystem", perf::performanceSubsystemName(
                         overlay.latest_spike->dominant_subsystem)},
                     {"dominant_time_us", overlay.latest_spike->dominant_time_us}};
        }
        m_snapshot = {{"panel", "perf_diagnostics"}, {"status", "live"},
                      {"revision", m_live_snapshot->revision}, {"session_id", m_live_snapshot->session_id},
                      {"capturing", overlay.capturing}, {"captured_frames", overlay.captured_frames},
                      {"latest_frame_us", overlay.latest_frame_us}, {"average_frame_us", overlay.average_frame_us},
                      {"maximum_frame_us", overlay.maximum_frame_us},
                      {"memory_bytes", overlay.latest_memory_bytes}, {"subsystems", std::move(subsystems)},
                      {"latest_spike", std::move(spike)},
                      {"last_control_code", m_live_snapshot->last_control_code}};
        return;
    }
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
