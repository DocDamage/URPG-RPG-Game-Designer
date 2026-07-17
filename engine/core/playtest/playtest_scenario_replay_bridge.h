#pragma once

#include "engine/core/replay/scenario_replay.h"

#include <filesystem>
#include <functional>
#include <optional>

namespace urpg::playtest {

struct ScenarioReplayControl {
    uint64_t control_id = 0;
    uint64_t expected_revision = 0;
    replay::ReplayExecutionMode mode = replay::ReplayExecutionMode::Headless;
};

struct LiveScenarioReplaySnapshot {
    uint64_t revision = 0;
    std::string session_id;
    bool capturing = false;
    replay::ReplayArtifact artifact;
    uint64_t last_control_id = 0;
    replay::ReplayExecutionResult last_result;
};

struct ScenarioReplayControlPollResult {
    size_t processed = 0;
    size_t applied = 0;
    size_t rejected = 0;
    bool io_error = false;
    std::string error;
};

class PlaytestScenarioReplayBridge {
public:
    using ControlHandler = std::function<bool(const ScenarioReplayControl&)>;

    explicit PlaytestScenarioReplayBridge(std::filesystem::path session_directory)
        : session_directory_(std::move(session_directory)) {}

    bool publish(const LiveScenarioReplaySnapshot& snapshot, std::string* diagnostic = nullptr) const;
    std::optional<LiveScenarioReplaySnapshot> readAfter(uint64_t revision,
                                                        std::string* diagnostic = nullptr) const;
    bool appendControl(const ScenarioReplayControl& control, std::string* diagnostic = nullptr) const;
    ScenarioReplayControlPollResult pollControls(const ControlHandler& handler, size_t max_controls = 4);

private:
    std::filesystem::path session_directory_;
    uintmax_t consumed_control_bytes_ = 0;
};

} // namespace urpg::playtest
