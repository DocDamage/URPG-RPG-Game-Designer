#include "editor/puzzle/puzzle_panel.h"

namespace urpg::editor::puzzle {

std::string PuzzlePanel::snapshot(const urpg::puzzle::PuzzleState& state) {
    if (state.solved) {
        return "puzzle:solved";
    }
    return state.failed ? "puzzle:failed" : "puzzle:active";
}

} // namespace urpg::editor::puzzle
