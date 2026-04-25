#include "engine/core/replay/replay_player.h"

#include <set>
#include <utility>

namespace urpg::replay {

ReplayPlayer::ReplayPlayer(std::size_t max_checkpoints) : max_checkpoints_(max_checkpoints) {}

void ReplayPlayer::captureCheckpoint(int64_t tick, nlohmann::json state) {
    checkpoints_[tick] = std::move(state);
    while (checkpoints_.size() > max_checkpoints_) {
        checkpoints_.erase(checkpoints_.begin());
    }
}

std::optional<nlohmann::json> ReplayPlayer::restoreCheckpoint(int64_t tick) const {
    const auto it = checkpoints_.find(tick);
    if (it == checkpoints_.end()) {
        return std::nullopt;
    }
    return it->second;
}

ReplayComparison ReplayPlayer::compare(const ReplayArtifact& expected, const ReplayArtifact& actual) {
    std::set<int64_t> ticks;
    for (const auto& [tick, _] : expected.state_hashes) {
        ticks.insert(tick);
    }
    for (const auto& [tick, _] : actual.state_hashes) {
        ticks.insert(tick);
    }

    for (const auto tick : ticks) {
        const auto expected_it = expected.state_hashes.find(tick);
        const auto actual_it = actual.state_hashes.find(tick);
        const auto expected_hash = expected_it == expected.state_hashes.end() ? std::string{} : expected_it->second;
        const auto actual_hash = actual_it == actual.state_hashes.end() ? std::string{} : actual_it->second;
        if (expected_hash != actual_hash) {
            return ReplayComparison{false, tick, expected_hash, actual_hash};
        }
    }

    return ReplayComparison{};
}

} // namespace urpg::replay
