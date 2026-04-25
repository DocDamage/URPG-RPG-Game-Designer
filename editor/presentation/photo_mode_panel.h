#pragma once

#include "engine/core/presentation/photo_mode_state.h"

#include <string>

namespace urpg::editor::presentation {

class PhotoModePanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const urpg::presentation::PhotoModeState& state);
};

} // namespace urpg::editor::presentation
