#pragma once

#include <string>
#include <vector>

namespace urpg::ui {

struct UiThemeAssets {
    std::string windowFrame;
    int windowBorderPixels = 0;
    std::string buttonNormal;
    std::string buttonHover;
    std::string buttonPressed;
    std::string buttonDisabled;
    std::vector<std::string> spriteSheets;
    std::vector<std::string> animatedSprites;
};

struct UiTheme {
    std::string id;
    std::string font;
    std::string cursor;
    std::string menuSound;
    std::vector<std::string> screens;
    UiThemeAssets assets;
};

struct ThemePreviewSnapshot {
    std::string themeId;
    std::vector<std::string> screens;
    std::string stableHash;
};

class ThemeRegistry {
public:
    [[nodiscard]] static UiTheme modernUiTheme();

    void addTheme(UiTheme theme);
    [[nodiscard]] UiTheme theme(const std::string& id) const;
    [[nodiscard]] ThemePreviewSnapshot previewScreens(const std::string& theme_id, std::vector<std::string> screens) const;

private:
    std::vector<UiTheme> themes_;
};

} // namespace urpg::ui
