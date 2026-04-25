#include "editor/ui/theme_builder_panel.h"

namespace urpg::editor::ui {

std::string ThemeBuilderPanel::snapshotLabel(const urpg::ui::ThemePreviewSnapshot& snapshot) {
    return snapshot.screens.empty() ? "theme:empty" : "theme:preview-ready";
}

} // namespace urpg::editor::ui
