#include "editor/presentation/photo_mode_panel.h"

namespace urpg::editor::presentation {

std::string PhotoModePanel::snapshotLabel(const urpg::presentation::PhotoModeState& state) {
    if (!state.isActive()) {
        return "photo:inactive";
    }
    return state.exportScreenshotState("").uiHidden ? "photo:active-hidden-ui" : "photo:active";
}

} // namespace urpg::editor::presentation
