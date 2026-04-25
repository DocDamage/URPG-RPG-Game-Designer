#pragma once

#include "engine/core/ui/theme_registry.h"

#include <string>

namespace urpg::editor::ui {

class ThemeBuilderPanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const urpg::ui::ThemePreviewSnapshot& snapshot);
};

} // namespace urpg::editor::ui
