#pragma once

#include <string>
#include <vector>

namespace urpg::ui {

struct UiTheme {
    std::string id;
    std::string font;
    std::string cursor;
    std::string menuSound;
    std::vector<std::string> screens;
};

struct ThemePreviewSnapshot {
    std::string themeId;
    std::vector<std::string> screens;
    std::string stableHash;
};

class ThemeRegistry {
public:
    void addTheme(UiTheme theme);
    [[nodiscard]] UiTheme theme(const std::string& id) const;
    [[nodiscard]] ThemePreviewSnapshot previewScreens(const std::string& theme_id, std::vector<std::string> screens) const;

private:
    std::vector<UiTheme> themes_;
};

} // namespace urpg::ui
