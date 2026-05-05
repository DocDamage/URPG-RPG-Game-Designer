#include "engine/core/ui/theme_registry.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace urpg::ui {

UiTheme ThemeRegistry::modernUiTheme() {
    const std::vector<std::string> sheets{
        "content/ui/modern/modern_ui_style_1_16x16.png",
        "content/ui/modern/modern_ui_style_2_16x16.png",
        "content/ui/modern/modern_ui_gamepad_16x16.png",
        "content/ui/modern/modern_ui_style_1_32x32.png",
        "content/ui/modern/modern_ui_style_2_32x32.png",
        "content/ui/modern/modern_ui_gamepad_32x32.png",
        "content/ui/modern/modern_ui_style_1_48x48.png",
        "content/ui/modern/modern_ui_style_2_48x48.png",
        "content/ui/modern/modern_ui_gamepad_48x48.png",
    };
    const std::vector<std::string> animatedSprites{
        "content/ui/modern/animated/modern_ui_button_trash_16x16_1.gif",
        "content/ui/modern/animated/modern_ui_button_trash_16x16_2.gif",
        "content/ui/modern/animated/modern_ui_button_trash_16x16_3.gif",
        "content/ui/modern/animated/modern_ui_button_trash_16x16_4.gif",
        "content/ui/modern/animated/modern_ui_button_trash_16x16_5.gif",
        "content/ui/modern/animated/modern_ui_button_trash_16x16_6.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_1.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_2.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_3.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_4.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_5.gif",
        "content/ui/modern/animated/modern_ui_button_trash_32x32_6.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_1.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_2.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_3.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_4.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_5.gif",
        "content/ui/modern/animated/modern_ui_button_trash_48x48_6.gif",
    };
    return {
        "theme.modern_ui",
        "default.ttf",
        "cursor.default",
        "ok.ogg",
        {"title", "menu", "battle"},
        {
            "content/ui/modern/modern_ui_style_1_48x48.png",
            48,
            "content/ui/modern/modern_ui_style_1_48x48.png",
            "content/ui/modern/modern_ui_style_1_48x48.png",
            "content/ui/modern/modern_ui_style_2_48x48.png",
            "content/ui/modern/modern_ui_style_2_48x48.png",
            sheets,
            animatedSprites,
        },
    };
}

void ThemeRegistry::addTheme(UiTheme theme) {
    themes_.push_back(std::move(theme));
}

UiTheme ThemeRegistry::theme(const std::string& id) const {
    const auto it = std::ranges::find_if(themes_, [&](const UiTheme& theme) { return theme.id == id; });
    return it == themes_.end() ? UiTheme{} : *it;
}

ThemePreviewSnapshot ThemeRegistry::previewScreens(const std::string& theme_id, std::vector<std::string> screens) const {
    std::ranges::sort(screens);
    std::ostringstream hash;
    hash << theme_id << ":";
    for (std::size_t i = 0; i < screens.size(); ++i) {
        if (i > 0) {
            hash << "|";
        }
        hash << screens[i];
    }
    return {theme_id, screens, hash.str()};
}

} // namespace urpg::ui
