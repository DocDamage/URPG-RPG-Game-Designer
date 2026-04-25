#pragma once

#include "engine/core/replay/replay_recorder.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::replay {

enum class GoldenReplayStatus : uint8_t {
    Passed,
    Diverged,
    MissingActual,
    MissingExpected,
    StaleProjectVersion
};

struct GoldenReplayResult {
    std::string id;
    GoldenReplayStatus status = GoldenReplayStatus::Passed;
    int64_t first_mismatched_tick = -1;
    std::string expected_hash;
    std::string actual_hash;
    std::string message;
};

struct GoldenReplayReport {
    bool passed = true;
    std::vector<GoldenReplayResult> results;
};

class GoldenReplayLane {
public:
    static GoldenReplayReport compare(const std::vector<ReplayArtifact>& expected,
                                      const std::vector<ReplayArtifact>& actual,
                                      const std::string& current_project_version);
};

std::string toString(GoldenReplayStatus status);

} // namespace urpg::replay
