#pragma once

#include <map>
#include <string>
#include <vector>

namespace urpg::puzzle {

struct PuzzleDefinition {
    std::string id;
    std::vector<std::string> ordered_triggers;
    bool reset_on_fail{true};
    std::string reward_flag;
};

struct PuzzleState {
    std::size_t progress{0};
    bool failed{false};
    bool solved{false};
};

struct PuzzleTriggerResult {
    bool solved{false};
    std::string rewardFlag;
};

class PuzzleRegistry {
public:
    void addPuzzle(PuzzleDefinition puzzle);
    [[nodiscard]] PuzzleTriggerResult trigger(const std::string& puzzle_id, const std::string& trigger_id);
    void reset(const std::string& puzzle_id);
    [[nodiscard]] PuzzleState state(const std::string& puzzle_id) const;

private:
    std::vector<PuzzleDefinition> puzzles_;
    std::map<std::string, PuzzleState> states_;
};

} // namespace urpg::puzzle
