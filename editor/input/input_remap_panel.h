#pragma once

#include "engine/core/input/input_remap_profile.h"

#include <string>

namespace urpg::editor::input {

class InputRemapPanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const urpg::input::InputRemapProfile& profile, const std::string& control);
};

} // namespace urpg::editor::input
