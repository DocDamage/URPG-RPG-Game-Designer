#include "editor/input/input_remap_panel.h"

namespace urpg::editor::input {

std::string InputRemapPanel::snapshotLabel(const urpg::input::InputRemapProfile& profile, const std::string& control) {
    return profile.bindingsFor(control).empty() ? "input:empty" : "input:bound";
}

} // namespace urpg::editor::input
