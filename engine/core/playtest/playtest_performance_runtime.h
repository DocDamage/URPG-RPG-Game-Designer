#pragma once

#include "engine/core/playtest/playtest_performance_bridge.h"

namespace urpg::playtest {

class PlaytestPerformanceRuntime {
public:
    explicit PlaytestPerformanceRuntime(std::filesystem::path session_directory,
                                        perf::PerformanceCaptureConfig config = {})
        : bridge_(std::move(session_directory)), capture_(config) {}

    bool start(std::string session_id, std::string* diagnostic = nullptr);
    bool record(perf::PerformanceFrameSample sample, std::string* diagnostic = nullptr);
    bool poll(std::string* diagnostic = nullptr);

    uint64_t revision() const { return revision_; }
    const perf::PerformanceCapture& capture() const { return capture_; }

private:
    bool publish(std::string* diagnostic);

    PlaytestPerformanceBridge bridge_;
    perf::PerformanceCapture capture_;
    std::string session_id_;
    uint64_t revision_ = 0;
    uint64_t last_control_id_ = 0;
    std::string last_control_code_;
};

} // namespace urpg::playtest
