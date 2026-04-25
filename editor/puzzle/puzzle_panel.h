#pragma once

#include "engine/core/puzzle/puzzle_registry.h"

#include <string>

namespace urpg::editor::puzzle {

class PuzzlePanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::puzzle::PuzzleState& state);
};

} // namespace urpg::editor::puzzle
