#include "engine/core/puzzle/puzzle_registry.h"

#include <algorithm>

namespace urpg::puzzle {

void PuzzleRegistry::addPuzzle(PuzzleDefinition puzzle) {
    states_[puzzle.id] = {};
    puzzles_.push_back(std::move(puzzle));
}

PuzzleTriggerResult PuzzleRegistry::trigger(const std::string& puzzle_id, const std::string& trigger_id) {
    auto puzzle = std::ranges::find_if(puzzles_, [&](const PuzzleDefinition& definition) { return definition.id == puzzle_id; });
    if (puzzle == puzzles_.end()) {
        return {};
    }
    auto& current = states_[puzzle_id];
    if (current.solved) {
        return {true, puzzle->reward_flag};
    }
    if (current.progress >= puzzle->ordered_triggers.size() || puzzle->ordered_triggers[current.progress] != trigger_id) {
        current.failed = true;
        if (puzzle->reset_on_fail) {
            current.progress = 0;
        }
        return {};
    }
    ++current.progress;
    current.solved = current.progress == puzzle->ordered_triggers.size();
    return {current.solved, current.solved ? puzzle->reward_flag : std::string{}};
}

void PuzzleRegistry::reset(const std::string& puzzle_id) {
    states_[puzzle_id] = {};
}

PuzzleState PuzzleRegistry::state(const std::string& puzzle_id) const {
    return states_.contains(puzzle_id) ? states_.at(puzzle_id) : PuzzleState{};
}

} // namespace urpg::puzzle
