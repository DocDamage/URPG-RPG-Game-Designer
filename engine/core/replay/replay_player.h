#pragma once

#include "engine/core/replay/replay_recorder.h"

#include <cstddef>
#include <map>
#include <optional>

namespace urpg::replay {

struct ReplayComparison {
    bool matches = true;
    int64_t first_mismatched_tick = -1;
    std::string expected_hash;
    std::string actual_hash;
};

class ReplayPlayer {
public:
    explicit ReplayPlayer(std::size_t max_checkpoints = 16);

    void captureCheckpoint(int64_t tick, nlohmann::json state);
    std::optional<nlohmann::json> restoreCheckpoint(int64_t tick) const;

    static ReplayComparison compare(const ReplayArtifact& expected, const ReplayArtifact& actual);

private:
    std::size_t max_checkpoints_ = 16;
    std::map<int64_t, nlohmann::json> checkpoints_;
};

} // namespace urpg::replay
