#include "engine/core/playtest/playtest_performance_runtime.h"

namespace urpg::playtest {

bool PlaytestPerformanceRuntime::start(std::string session_id, std::string* diagnostic) {
    if (session_id.empty()) return false;
    session_id_ = std::move(session_id);
    const auto started = capture_.start(session_id_ + "-live");
    last_control_code_ = started.code;
    return started.success && publish(diagnostic);
}

bool PlaytestPerformanceRuntime::record(perf::PerformanceFrameSample sample, std::string* diagnostic) {
    if (!capture_.overlay().capturing) return true;
    const auto before_spike = capture_.spikes().size();
    const auto recorded = capture_.record(std::move(sample));
    if (!recorded.success) {
        if (diagnostic) *diagnostic = recorded.code;
        return false;
    }
    const auto frame_count = capture_.frames().size();
    if (capture_.spikes().size() != before_spike || frame_count == 1 || frame_count % 10 == 0) {
        return publish(diagnostic);
    }
    return true;
}

bool PlaytestPerformanceRuntime::poll(std::string* diagnostic) {
    const auto result = bridge_.pollControls([&](const PerformanceControl& control) {
        last_control_id_ = control.control_id;
        if (control.expected_revision != revision_) {
            last_control_code_ = "performance_control_stale_revision";
            return;
        }
        const auto outcome = control.action == PerformanceControlAction::Start
                                 ? capture_.start(control.capture_id) : capture_.stop();
        last_control_code_ = outcome.code;
    });
    if (result.io_error) {
        if (diagnostic) *diagnostic = result.error;
        return false;
    }
    if (result.processed > 0) return publish(diagnostic);
    return true;
}

bool PlaytestPerformanceRuntime::publish(std::string* diagnostic) {
    ++revision_;
    return bridge_.publish({revision_, session_id_, capture_.overlay(), capture_.report(),
                            last_control_id_, last_control_code_}, diagnostic);
}

} // namespace urpg::playtest
