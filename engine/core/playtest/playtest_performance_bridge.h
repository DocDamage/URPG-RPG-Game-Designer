#pragma once

#include "engine/core/perf/performance_capture.h"

#include <filesystem>
#include <functional>
#include <optional>

namespace urpg::playtest {

enum class PerformanceControlAction : uint8_t { Start, Stop };

struct PerformanceControl {
    uint64_t control_id = 0;
    uint64_t expected_revision = 0;
    PerformanceControlAction action = PerformanceControlAction::Start;
    std::string capture_id;
};

struct LivePerformanceSnapshot {
    uint64_t revision = 0;
    std::string session_id;
    perf::PerformanceOverlaySnapshot overlay;
    nlohmann::json report = nlohmann::json::object();
    uint64_t last_control_id = 0;
    std::string last_control_code;
};

struct PerformanceControlPollResult {
    size_t processed = 0;
    bool io_error = false;
    std::string error;
};

class PlaytestPerformanceBridge {
public:
    using ControlHandler = std::function<void(const PerformanceControl&)>;
    explicit PlaytestPerformanceBridge(std::filesystem::path session_directory)
        : session_directory_(std::move(session_directory)) {}

    bool publish(const LivePerformanceSnapshot& snapshot, std::string* diagnostic = nullptr) const;
    std::optional<LivePerformanceSnapshot> readAfter(uint64_t revision,
                                                     std::string* diagnostic = nullptr) const;
    bool appendControl(const PerformanceControl& control, std::string* diagnostic = nullptr) const;
    PerformanceControlPollResult pollControls(const ControlHandler& handler, size_t max_controls = 4);

private:
    std::filesystem::path session_directory_;
    uintmax_t consumed_control_bytes_ = 0;
};

} // namespace urpg::playtest
